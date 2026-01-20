#include "VCA810.h"

// 记录当前放大倍数
static float s_current_factor = 1.0f;
// 记录当前档位
static VCA_Level_t s_current_level = VCA_GAIN_0DB;

typedef struct {
    uint16_t dac_mv;   // DAC控制电压
    float    factor;   // 实际放大倍数
} Gain_Config_t;

/* * 增益表配置 
 * 输入范围 60mV ~ 1000mV
 * 放大模块控制：0mV~2000mV 对应增益 -42dB~32dB （正相关）
 * 目标: ADC输入控制在 2.0Vpp 左右 (最大不能超过 3.0Vpp)
 * 增益曲线：需要实际测量
 */
static const Gain_Config_t Gain_Table[] = {
    [VCA_GAIN_MIN_N40DB] = {200,  0.01f},   // 几乎关断
    [VCA_GAIN_0DB]       = {1180,  0.0f},   //直通，增益0dB
    [VCA_GAIN_LoW]       = {1350, 2.10f},   //500-1000mV适用 放大2.1倍
    [VCA_GAIN_MIDDLE]    = {1460, 3.70f},   //200-500mV适用 放大3.7倍
    [VCA_GAIN_HIGH]      = {1600, 7.60f},   //60-200mV适用 放大7.6倍
    [VCA_GAIN_MAX_30DB]  = {1950, 31.6f}    // 31.6倍 
};
/*初始化模块并默认挡位为直通*/
void VCA810_Init(void) {
    DL_DAC12_enable(VCA_DAC_INST);
    // 默认给一个中间增益，防止上电信号过大
    VCA810_SetGain(VCA_GAIN_0DB); 
}

/*设置输出电压,理论范围0~3.3V，上限限制到了2.5v(2500mV)*/
void VCA810_SetVoltage_mV(uint16_t mv) {
    if(mv > 2500) mv = 2500; // 保护限制
    
    // MSPM0 DAC: Vref=3.3V(VDD), 12bit
    uint32_t code = (uint32_t)mv * 4095 / 3300;
    if(code > 4095) code = 4095;
    
    DL_DAC12_output12(VCA_DAC_INST, code);
}

/*设置增益挡位*/
void VCA810_SetGain(VCA_Level_t level) {
    if(level > VCA_GAIN_MAX_30DB) level = VCA_GAIN_MAX_30DB;
    
    s_current_level = level;
    s_current_factor = Gain_Table[level].factor;
    
    VCA810_SetVoltage_mV(Gain_Table[level].dac_mv);
}
/*获取当前具体增益数值*/
float VCA810_GetGainFactor(void) {
    return s_current_factor;
}