/* fun.h */
#ifndef FUN_H
#define FUN_H

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

//------------------------
//  
//
//------------------------
// 系统常数
#define ADC_VREF        3.3f    
#define ADC_BITS        4096.0f 
#define BIAS_VOLTAGE    1.65f   //直流偏置电压，需要实际值 
#define FFT_LENGTH      256     //fft采样点    
//
#define ms_cycle        (80000)
// 波形类型
typedef enum {
    WAVE_UNKNOWN = 0,
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE
} Wave_Type_t;

// 信号分析结果
typedef struct {
    float vpp_adc;      // ADC端Vpp
    float vpp_real_mv;  // 真实Vpp (mV)
    float freq_hz;      // 频率
    Wave_Type_t type;   // 波形
} Signal_Info_t;

// --- API ---
/**
 * @name:LED调试
 * @note:count为闪烁次数 interval_ms间隔秒数（ms）
 */
void LED_Debug(uint8_t count, uint32_t interval_ms);



void Fun_Init(void);
void Fun_Start_Sampling(void); // 启动新一轮采集
bool Fun_Is_Data_Ready(void);  // 查询是否采集完成
void Fun_Process(void);        // 数据处理主入口
Signal_Info_t Fun_Get_Result(void); // 获取结果


// 中断处理接口 (供 main.c 调用)
void Fun_DMA_ADC_Handler(void);     // ADC数据搬完中断
void Fun_Freq_Capture_Handler(void);// 频率计捕获中断

#endif /* FUN_H */