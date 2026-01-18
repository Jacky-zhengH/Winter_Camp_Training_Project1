#ifndef VCA810_H
#define VCA810_H

#include "ti_msp_dl_config.h"

// 宏定义
#define VCA_DAC_INST        DAC0 

// 增益档位枚举
typedef enum {
    VCA_GAIN_MIN_N40DB = 0, // -40dB (关断/极小)
    VCA_GAIN_N6DB,          // -6dB  (0.5x)
    VCA_GAIN_0DB,           //  0dB  (1.0x)
    VCA_GAIN_10DB,          // +10dB (3.16x)
    VCA_GAIN_14DB,          // +14dB (5.0x)  <-- 新增中间档位，适配 60-1000mV 范围
    VCA_GAIN_20DB,          // +20dB (10.0x)
    VCA_GAIN_MAX_30DB       // +30dB (31.6x)
} VCA_Level_t;

void VCA810_Init(void);
void VCA810_SetGain(VCA_Level_t level);
float VCA810_GetGainFactor(void);
void VCA810_SetVoltage_mV(uint16_t mv);

#endif /* VCA810_H */