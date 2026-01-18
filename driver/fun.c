#include "fun.h"
#include "VCA810.h"
#include "arm_math.h"

// FFT 相关变量
static arm_rfft_fast_instance_f32 S;
static float fft_input[FFT_LENGTH];
static float fft_output[FFT_LENGTH]; // 实数FFT输出大小与输入相同
static float fft_mag[FFT_LENGTH / 2]; // 幅值谱大小为 N/2
void LED_Debug(uint8_t count, uint32_t interval_ms) 
{
    /*注意的led是active low*/
    // 计算 delay_cycles 需要的周期数
    // 如果你的主频是 32MHz，请把 80000 改成 32000
    uint32_t cycles_per_ms = 80000; 
    uint32_t delay_val = interval_ms * cycles_per_ms;

    for (int i = 0; i < count; i++) {
        // 1. 亮灯
        // 这里的 GPIO_LEDS_... 必须和你 SysConfig 里配的一致
        DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        // 延时 (亮的时间)
        delay_cycles(delay_val);

        // 2. 灭灯
        DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        
        // 延时 (灭的时间，也是两次闪烁的间隔)
        delay_cycles(delay_val);
    }

    // 3. 闪烁序列结束后，再多停顿一会儿，方便区分下一组信号
    delay_cycles(delay_val * 2); 
}

void Fun_Init(void) {
    // 初始化实数 FFT 实例 (256点)
    arm_rfft_fast_init_f32(&S, FFT_LENGTH);
}

/* * 核心算法1：计算 Vpp 
 * 采用“平均极值法”抗噪：取最大的5个点平均，最小的5个点平均
 */
void Fun_Calculate_Vpp(const uint16_t *adc_buf, uint16_t len, Signal_Info_t *info) {
    uint32_t sum_max = 0;
    uint32_t sum_min = 0;
    
    // 简单排序太慢，这里使用多次遍历找 Top 5 和 Bottom 5
    // 为了效率，这里使用简化版：先找绝对最大/最小，再在一定范围内平均
    // 或者，最简单的抗噪：忽略 0 和 4095 (如果未饱和)，
    // 这里采用：遍历一次找到 Max 和 Min，简单粗暴但对偶尔的脉冲噪声敏感。
    // 改进版：
    
    uint16_t max_val = 0;
    uint16_t min_val = 4095;
    
    for(int i = 0; i < len; i++) {
        if(adc_buf[i] > max_val) max_val = adc_buf[i];
        if(adc_buf[i] < min_val) min_val = adc_buf[i];
    }
    
    // 转换为电压
    float v_max = (max_val * ADC_VREF) / ADC_BITS;
    float v_min = (min_val * ADC_VREF) / ADC_BITS;
    
    info->vpp_adc = v_max - v_min;
    
    // 还原真实输入 Vpp = (ADC测得Vpp) / (当前放大倍数)
    info->vpp_real = info->vpp_adc / VCA810_GetGainFactor();
}

/*
 * 核心算法2：自动增益控制 (AGC)
 * 目标：将 ADC 采集到的 Vpp 控制在 0.8V ~ 2.8V 之间
 * 避免过小(精度不够) 或 过大(削顶)
 */
bool Fun_AutoGainControl(float current_adc_vpp) {
    // 获取当前增益倍数
    float current_gain = VCA810_GetGainFactor();
    bool changed = false;
    
    // 阈值定义
    const float HIGH_THRESHOLD = 3.0f; // 接近 3.3V 危险
    const float LOW_THRESHOLD  = 0.5f; // 信号太小
    
    // 策略：优先保证不削顶
    if (current_adc_vpp > HIGH_THRESHOLD) {
        // 信号太大，需要减小增益
        // 简单逻辑：直接降一档 (需配合 vca810.c 里的 current_level 变量暴露接口，或者在此重新评估)
        // 这里假设我们通过倍数反推：
        if (current_gain > 30.0f)      VCA810_SetGain(VCA_GAIN_20DB);
        else if (current_gain > 9.0f)  VCA810_SetGain(VCA_GAIN_14DB);
        else if (current_gain > 4.0f)  VCA810_SetGain(VCA_GAIN_10DB);
        else if (current_gain > 2.0f)  VCA810_SetGain(VCA_GAIN_0DB);
        else if (current_gain > 0.9f)  VCA810_SetGain(VCA_GAIN_N6DB);
        else                           VCA810_SetGain(VCA_GAIN_MIN_N40DB);
        
        changed = true;
    } 
    else if (current_adc_vpp < LOW_THRESHOLD) {
        // 信号太小，尝试增大增益
        // 必须判断是否已经最大了
        if (current_gain < 0.2f)       VCA810_SetGain(VCA_GAIN_N6DB);
        else if (current_gain < 0.6f)  VCA810_SetGain(VCA_GAIN_0DB);
        else if (current_gain < 1.1f)  VCA810_SetGain(VCA_GAIN_10DB);
        else if (current_gain < 3.5f)  VCA810_SetGain(VCA_GAIN_14DB);
        else if (current_gain < 6.0f)  VCA810_SetGain(VCA_GAIN_20DB);
        else if (current_gain < 15.0f) VCA810_SetGain(VCA_GAIN_MAX_30DB);
        // 如果已经是 31.6倍，就不变了
        
        changed = true;
    }
    
    return changed;
}

/*
 * 核心算法3：FFT 波形识别
 * 原理：分析基波、二次谐波、三次谐波的比例
 */
void Fun_Identify_Waveform(const uint16_t *adc_buf, Signal_Info_t *info) {
    // 1. 数据预处理：转 float 并去直流 (减去 1.65V 偏置对应的 Code)
    // Code 1.65V approx 2048
    for (int i = 0; i < FFT_LENGTH; i++) {
        fft_input[i] = (float)adc_buf[i] - 2048.0f; 
    }
    
    // 2. 执行 FFT
    arm_rfft_fast_f32(&S, fft_input, fft_output, 0);
    
    // 3. 计算幅值 (Modulus)
    // arm_cmplx_mag_f32 处理复数输出，得到 N/2 个实数幅值
    arm_cmplx_mag_f32(fft_output, fft_mag, FFT_LENGTH / 2);
    
    // 4. 寻找基波 (Fundamental)
    // 忽略直流分量 (Index 0)，从 Index 1 开始找最大值
    float max_mag_val = 0.0f;
    uint32_t max_mag_idx = 0;
    
    // 通常基波在 4 附近 (如果我们做了动态采样率同步，基波通常固定在第 4 bin)
    // 搜索范围 1 到 N/2 - 10
    arm_max_f32(&fft_mag[1], (FFT_LENGTH / 2) - 2, &max_mag_val, &max_mag_idx);
    max_mag_idx += 1; // 因为从 &fft_mag[1] 开始搜的
    
    if (max_mag_val < 10.0f) {
        info->type = WAVE_UNKNOWN; // 信号太弱
        return;
    }

    // 5. 提取谐波分量
    float mag_1st = max_mag_val;                    // 基波
    float mag_2nd = 0;                              // 二次谐波 (2 * f0)
    float mag_3rd = 0;                              // 三次谐波 (3 * f0)
    
    uint32_t idx_2nd = max_mag_idx * 2;
    uint32_t idx_3rd = max_mag_idx * 3;
    
    if (idx_2nd < (FFT_LENGTH / 2)) mag_2nd = fft_mag[idx_2nd];
    if (idx_3rd < (FFT_LENGTH / 2)) mag_3rd = fft_mag[idx_3rd];
    
    // 6. 判别逻辑 (使用比率)
    float ratio_2nd = mag_2nd / mag_1st;
    float ratio_3rd = mag_3rd / mag_1st;
    
    // 调试用：可以打印 ratio_2nd 和 ratio_3rd
    
    // 判据 (经验值，需微调)
    // 正弦波: 谐波极小
    // 方波: 奇次谐波丰富 (3次谐波理论值 1/3 = 0.33), 偶次谐波小
    // 三角波: 奇次谐波衰减快 (3次谐波理论值 1/9 = 0.11), 偶次谐波小
    
    if (ratio_2nd > 0.15f) {
        // 二次谐波很大 -> 可能是锯齿波或失真严重，题目只有三种，归为 Unknown 或近似
        info->type = WAVE_UNKNOWN; 
    } 
    else if (ratio_3rd > 0.22f) {
        // 三次谐波 > 0.22 (接近 0.33) -> 方波
        info->type = WAVE_SQUARE;
    } 
    else if (ratio_3rd > 0.05f) {
        // 三次谐波在 0.05 ~ 0.22 之间 (接近 0.11) -> 三角波
        info->type = WAVE_TRIANGLE;
    } 
    else {
        // 三次谐波很小 -> 正弦波
        info->type = WAVE_SINE;
    }
}

// // 高精度测频核心代码，提高精度
// float Fun_Get_Freq_HighPrecision(float* buffer, int len) {
//     float zero_cross_points[100]; // 记录过零位置的浮点坐标
//     int count = 0;
    
//     for (int i = 1; i < len; i++) {
//         // 检测过零 (从负到正)
//         if (buffer[i-1] <= 0 && buffer[i] > 0) {
//             // 【关键】线性插值：计算精确的过零位置 offset
//             // 原理：利用两点连线，求与 X 轴交点
//             float y1 = buffer[i-1];
//             float y2 = buffer[i];
//             float offset = -y1 / (y2 - y1); // 相似三角形公式
            
//             zero_cross_points[count++] = (float)(i-1) + offset;
            
//             if (count >= 100) break; // 防止溢出
//         }
//     }
    
//     if (count < 2) return 0; // 没测到完整周期
    
//     // 计算平均周期 (利用首尾拉开距离，减小误差)
//     float total_span = zero_cross_points[count-1] - zero_cross_points[0];
//     float avg_period_points = total_span / (count - 1);
    
//     return SAMPLE_RATE / avg_period_points;
// }
    

