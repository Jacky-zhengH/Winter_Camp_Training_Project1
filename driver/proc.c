/* proc.c */
#include "proc.h"
#include "VCA810.h"
#include "fun.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define UART_TX_BUF_SIZE 256
static uint8_t s_tx_buffer[UART_TX_BUF_SIZE];
static volatile bool s_is_tx_busy = false;

static void UART_DMA_Start(uint8_t *buf, uint16_t len);

void Proc_Init(void) {
    // 设置 DMA UART TX 目标地址 (永远不变)
    DL_DMA_setDestAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t)(&UART_0_INST->TXDATA));
    s_is_tx_busy = false;
    
    // 启动第一次采样
    Fun_Start_Sampling();
}

void Proc_Task(void) {
    // 轮询：数据是否准备好？
    if (Fun_Is_Data_Ready()) {
        // 1. 获取本次采样前的 AGC 状态
        float old_gain = VCA810_GetGainFactor();
        
        // 2. 处理数据
        Fun_Process(); // 内部会计算 Vpp, AGC, FFT
        Signal_Info_t res = Fun_Get_Result();
        
        // 3. 检查 AGC 是否发生了变化
        if (VCA810_GetGainFactor() != old_gain) {
            // 增益变了，数据无效，立即重启采样
            Fun_Start_Sampling();
            return;
        }

        // 4. 发送结果到屏幕
        // 映射波形字符串
        char *type_str = "UNK";
        if(res.type == WAVE_SINE) type_str = "SIN";
        else if(res.type == WAVE_SQUARE) type_str = "SQU";
        else if(res.type == WAVE_TRIANGLE) type_str = "TRI";

        // 发送: "F:1000.0Hz V:500mV SIN"
        Proc_Printf("t0.txt=\"F:%.1fHz V:%.0fmV %s\"", res.freq_hz, res.vpp_real_mv, type_str);
        
        // 发送结束符 (Nextion/TJC)
        // 注意：如果上一条 printf 还没发完，这里会直接返回，可能导致丢包
        // 实际应用建议把结束符并在 printf 里，或者等待
        // 这里简单延时一下确保 DMA 启动
        delay_cycles(1000); 
        static uint8_t end_cmd[] = {0xFF, 0xFF, 0xFF};
        UART_DMA_Start(end_cmd, 3);
        
        // 5. 调试闪灯
        // LED_Debug(1, 50); 

        // 6. 启动下一轮
        Fun_Start_Sampling();
    }
}

// --- UART DMA 发送逻辑 ---

void Proc_Printf(const char *format, ...) {
    if (s_is_tx_busy) return; // 如果忙，丢弃
    
    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)s_tx_buffer, UART_TX_BUF_SIZE, format, args);
    va_end(args);
    
    if (len > 0) {
        UART_DMA_Start(s_tx_buffer, (uint16_t)len);
    }
}

static void UART_DMA_Start(uint8_t *buf, uint16_t len) {
    // 等待上一次 DMA 确实结束 (双重保险)
    while(s_is_tx_busy);
    
    s_is_tx_busy = true;
    DL_DMA_setSrcAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t)buf);
    DL_DMA_setTransferSize(DMA, DMA_CH2_CHAN_ID, len);
    DL_DMA_enableChannel(DMA, DMA_CH2_CHAN_ID);
}

void Proc_UART_Tx_Callback(void) {
    s_is_tx_busy = false;
}