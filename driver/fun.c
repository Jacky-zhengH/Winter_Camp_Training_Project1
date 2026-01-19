/* fun.c */
#include "fun.h"
#include "VCA810.h"
#include "arm_math.h"



// 1. 频率捕获定时器 (TimerA0) 的宏名
// 在 SysConfig 里可能是 TIMER_0_INST, CAPTURE_0_INST 等
#define TIMER_FREQ_INST      CAPTURE_0_INST  

// 2. ADC采样触发定时器 (TimerG0) 的宏名
// 在 SysConfig 里可能是 TIMER_1_INST, TIMER_ADC_TRIG_INST 等
// !! 千万不要和上面那个一样 !!
#define TIMER_SAMPLE_INST    TIMER_0_INST  

// ============================================================

// --- 全局变量 ---
// ADC Buffer (DMA 目标)
volatile uint16_t gADCBuffer[FFT_LENGTH]; 
// 标志位
static volatile bool s_data_ready = false;
static Signal_Info_t s_result = {0};

// FFT 相关
static arm_rfft_fast_instance_f32 S;
static float fft_input[FFT_LENGTH];
static float fft_output[FFT_LENGTH]; 
static float fft_mag[FFT_LENGTH / 2]; 

// 频率测量相关
static volatile float s_measured_freq = 0.0f;

// --- 内部函数声明 ---
static void Config_Sampling_Rate(float freq);
static void Fun_Calculate_Vpp(const uint16_t *adc_buf, uint16_t len, Signal_Info_t *info);
static bool Fun_AutoGainControl(float current_adc_vpp);
static void Fun_Identify_Waveform(const uint16_t *adc_buf, Signal_Info_t *info);

void Fun_Init(void) {
    arm_rfft_fast_init_f32(&S, FFT_LENGTH);
    // 初始化 DMA 目标地址
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&gADCBuffer[0]);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
}

void Fun_Start_Sampling(void) {
    s_data_ready = false;

    // 1. 获取当前频率 (如果没有测到频率，默认 1kHz)
    float current_freq = (s_measured_freq > 1.0f) ? s_measured_freq : 1000.0f;
    
    // 2. 动态调整 Timer Load 值 (操作的是 ADC 触发定时器：TimerG0)
    Config_Sampling_Rate(current_freq);
    
    // 3. 复位并使能 DMA
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&gADCBuffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, FFT_LENGTH);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
    
    // 4. 开启采样触发定时器 (操作的是 ADC 触发定时器：TimerG0)
    DL_TimerG_startCounter(TIMER_SAMPLE_INST);
}

bool Fun_Is_Data_Ready(void) {
    return s_data_ready;
}

Signal_Info_t Fun_Get_Result(void) {
    // 保证频率是最新测得的
    s_result.freq_hz = s_measured_freq;
    return s_result;
}

// === 核心处理流程 (Process) ===
void Fun_Process(void) {
    // 1. 计算 Vpp
    Fun_Calculate_Vpp((uint16_t*)gADCBuffer, FFT_LENGTH, &s_result);
    
    // 2. AGC 检查 (如果调整了增益，就退出，Proc层会重启采样)
    if (Fun_AutoGainControl(s_result.vpp_adc)) {
        return; 
    }
    
    // 3. 波形识别
    Fun_Identify_Waveform((uint16_t*)gADCBuffer, &s_result);
}

// === 频率测量中断处理 ===
// 使用 TimerA0 (FREQ_INST)
void Fun_Freq_Capture_Handler(void) {
    // [修正] 使用 TIMER_FREQ_INST
    // TimerA0 通常也是 GPTIMER 兼容，可以用 DL_TimerA 或 DL_TimerG 读取
    // 这里使用 DL_Timer (通用) 或 DL_TimerA 更准确
    uint32_t cap = DL_TimerA_getCaptureCompareValue(TIMER_FREQ_INST, DL_TIMER_CC_0_INDEX);
    
    // 清除计数器，准备下一次测量 (简单的频率计逻辑)
    DL_TimerA_setTimerCount(TIMER_FREQ_INST, 0); 
    
    // 计算频率 F = TIMG_CLK / Capture
    if (cap > 0) {
        // 假设 TimerA0 时钟是 80MHz (需确认 SysConfig)
        s_measured_freq = 80000000.0f / (float)cap; 
    }
}

// === DMA 完成中断处理 ===
void Fun_DMA_ADC_Handler(void) {
    // 采样完成，停止触发定时器 (操作的是 ADC 触发定时器：TimerG0)
    DL_TimerG_stopCounter(TIMER_SAMPLE_INST);
    s_data_ready = true;
}

// === 内部算法实现 ===

/*
 * 动态配置采样率：目标 Fs = 64 * Fin
 * 针对 80MHz 主频适配
 * 操作对象：TimerG0 (SAMPLE_INST)
 */
static void Config_Sampling_Rate(float freq) {
    // [修正] 使用 TIMER_SAMPLE_INST

    // 1. 必须先停止定时器才能修改时钟配置
    DL_TimerG_stopCounter(TIMER_SAMPLE_INST);

    // 2. 计算目标采样率
    // 策略：每周期采64点
    uint32_t target_fs = (uint32_t)(freq * 64.0f);
    
    // 3. 硬件限制钳位
    // 上限：2Msps (ADC极限)
    if (target_fs > 2000000) target_fs = 2000000; 
    // 下限：64Hz (对应 1Hz 信号)
    if (target_fs < 64) target_fs = 64;

    // 4. 计算需要的总计数值 (Total Ticks)
    // Bus Clock = 80MHz
    uint32_t timer_clock = 80000000; 
    uint32_t total_ticks_needed = timer_clock / target_fs;

    // 5. 准备配置结构体
    DL_Timer_ClockConfig clockConfig;
    clockConfig.clockSel = DL_TIMER_CLOCK_BUSCLK;
    clockConfig.divideRatio = DL_TIMER_CLOCK_DIVIDE_1; // 默认不分频(1分频)
    
    uint32_t load_val;
    uint8_t  calc_prescale = 0;

    // 6. 计算 Prescaler 和 Load Value
    // TIMG 是 16位定时器，Load Value 最大 65535
    if (total_ticks_needed <= 65535) {
        // 情况A: 不需要预分频，直接装载
        calc_prescale = 0;
        load_val = total_ticks_needed;
    } else {
        // 情况B: 需要预分频
        // total_ticks = (prescale + 1) * (load_val + 1)
        
        // 计算需要的最小分频系数 (向上取整)
        uint32_t divider = total_ticks_needed / 65535 + 1;
        
        // 限制 prescale 最大为 255 (即分频 256)
        if (divider > 256) divider = 256;
        
        calc_prescale = (uint8_t)(divider - 1);
        
        // 反推 Load Value
        load_val = total_ticks_needed / divider;
    }

    // 填入计算出的预分频值
    clockConfig.prescale = calc_prescale;

    // 7. 应用配置
    // 设置时钟源、分频、预分频
    DL_Timer_setClockConfig(TIMER_SAMPLE_INST, (DL_Timer_ClockConfig *) &clockConfig);
    
    // 设置计数值
    DL_TimerG_setLoadValue(TIMER_SAMPLE_INST, load_val);
    
    // 8. 重启定时器
    DL_TimerG_startCounter(TIMER_SAMPLE_INST);
}

static void Fun_Calculate_Vpp(const uint16_t *adc_buf, uint16_t len, Signal_Info_t *info) {
    uint16_t max_val = 0;
    uint16_t min_val = 4095;
    for(int i = 0; i < len; i++) {
        if(adc_buf[i] > max_val) max_val = adc_buf[i];
        if(adc_buf[i] < min_val) min_val = adc_buf[i];
    }
    
    float v_max = (max_val * ADC_VREF) / ADC_BITS;
    float v_min = (min_val * ADC_VREF) / ADC_BITS;
    info->vpp_adc = v_max - v_min;
    
    // 换算为真实 mV
    info->vpp_real_mv = (info->vpp_adc / VCA810_GetGainFactor()) * 1000.0f;
}

static bool Fun_AutoGainControl(float current_adc_vpp) {
    float gain = VCA810_GetGainFactor();
    bool changed = false;
    
    if (current_adc_vpp > 3.0f) { // 接近饱和
        if (gain > 30.0f)      VCA810_SetGain(VCA_GAIN_20DB);
        else if (gain > 9.0f)  VCA810_SetGain(VCA_GAIN_14DB);
        else if (gain > 4.0f)  VCA810_SetGain(VCA_GAIN_10DB);
        else if (gain > 2.0f)  VCA810_SetGain(VCA_GAIN_0DB);
        else if (gain > 0.9f)  VCA810_SetGain(VCA_GAIN_N6DB);
        else                   VCA810_SetGain(VCA_GAIN_MIN_N40DB);
        changed = true;
    } else if (current_adc_vpp < 0.5f) { // 信号过小
        if (gain < 0.2f)       VCA810_SetGain(VCA_GAIN_N6DB);
        else if (gain < 0.6f)  VCA810_SetGain(VCA_GAIN_0DB);
        else if (gain < 1.1f)  VCA810_SetGain(VCA_GAIN_10DB);
        else if (gain < 3.5f)  VCA810_SetGain(VCA_GAIN_14DB);
        else if (gain < 6.0f)  VCA810_SetGain(VCA_GAIN_20DB);
        else if (gain < 15.0f) VCA810_SetGain(VCA_GAIN_MAX_30DB);
        changed = true;
    }
    return changed;
}

static void Fun_Identify_Waveform(const uint16_t *adc_buf, Signal_Info_t *info) {
    // 1. 去直流预处理
    for (int i = 0; i < FFT_LENGTH; i++) {
        fft_input[i] = (float)adc_buf[i] - 2048.0f; 
    }
    // 2. FFT运算
    arm_rfft_fast_f32(&S, fft_input, fft_output, 0);
    arm_cmplx_mag_f32(fft_output, fft_mag, FFT_LENGTH / 2);
    
    // 3. 找基波和谐波
    float max_mag = 0.0f;
    uint32_t idx_1st = 0;
    // 忽略直流(index 0)，从1开始搜
    arm_max_f32(&fft_mag[1], (FFT_LENGTH/2)-2, &max_mag, &idx_1st);
    idx_1st += 1;
    
    float mag_2nd = 0, mag_3rd = 0;
    if (idx_1st * 2 < FFT_LENGTH/2) mag_2nd = fft_mag[idx_1st * 2];
    if (idx_1st * 3 < FFT_LENGTH/2) mag_3rd = fft_mag[idx_1st * 3];
    
    float r3 = mag_3rd / max_mag;
    
    if (r3 > 0.22f) info->type = WAVE_SQUARE;
    else if (r3 > 0.05f) info->type = WAVE_TRIANGLE;
    else info->type = WAVE_SINE;
}

void LED_Debug(uint8_t count, uint32_t interval_ms) {
    uint32_t cycles = interval_ms * 80000; // 假设80MHz
    for (int i = 0; i < count; i++) {
        DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        delay_cycles(cycles);
        DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        delay_cycles(cycles);
    }
    delay_cycles(cycles * 2); 
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
    

