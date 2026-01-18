#ifndef fun_h
#define fun_h

#include "ti_msp_dl_config.h"
#include "arm_math.h"

// 系统常数
#define ADC_VREF        3.3f    // V
#define ADC_BITS        4096.0f // 12-bit
#define BIAS_VOLTAGE    1.65f   // V

// FFT点数 (必须与 main.c 定义一致)
#define FFT_LENGTH      256

// 波形类型
typedef enum {
    WAVE_UNKNOWN = 0,
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE
} Wave_Type_t;

// 信号分析结果结构体
typedef struct {
    float vpp_adc;      // ADC端采集到的Vpp (V)
    float vpp_real;     // 换算回输入端的Vpp (V)
    float freq_hz;      // 频率 (由外部传入或FFT校准)
    Wave_Type_t type;   // 波形类型
} Signal_Info_t;

// --- API ---

// 初始化 DSP 库相关
void Fun_Init(void);

// 计算 Vpp (带抗噪)
void Fun_Calculate_Vpp(const uint16_t *adc_buf, uint16_t len, Signal_Info_t *info);

// 自动增益控制 (AGC) - 根据当前ADC幅度调整 VCA810
// 返回 true 表示增益已调整，建议丢弃当前数据重新采样
bool Fun_AutoGainControl(float current_adc_vpp);

// 波形识别 (FFT)
void Fun_Identify_Waveform(const uint16_t *adc_buf, Signal_Info_t *info);
/*led调试函数*/
void LED_Debug(uint8_t count, uint32_t interval_ms);

#endif/*function*/