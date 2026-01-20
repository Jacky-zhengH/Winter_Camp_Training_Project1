#ifndef VCA810_H
#define VCA810_H

#include "ti_msp_dl_config.h"

// 宏定义
#define VCA_DAC_INST        DAC0 
//输出引脚为                 PA15
// 增益档位枚举
typedef enum {
    VCA_GAIN_MIN_N40DB = 0,     // -40dB (关断/极小)
    VCA_GAIN_0DB,               //直通，增益0dB
    VCA_GAIN_LoW,               //500-1000mV适用 放大2.1倍
    VCA_GAIN_MIDDLE,            //200-500mV适用 放大3.7倍
    VCA_GAIN_HIGH,              //60-200mV适用 放大7.6倍
    VCA_GAIN_MAX_30DB           // +30dB (31.6x)
} VCA_Level_t;
//extern float s_current_factor;
//----------------------------------------------------
void VCA810_Init(void);
void VCA810_SetGain(VCA_Level_t level);
float VCA810_GetGainFactor(void);
void VCA810_SetVoltage_mV(uint16_t mv);

#endif /* VCA810_H */