/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "arm_math.h"
#include "ti_msp_dl_config.h"
#include "fun.h"
#include "proc.h"
#include "VCA810.h"


int main(void)
{
    SYSCFG_DL_init();
    VCA810_Init();
    LED_Debug(2, 200); 
    
    while (1) 
    {
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        delay_cycles(64000000); // 假设80MHz，这大概是0.5秒
    }
}

//----------------------------------------------------------
// === Interrupt Handlers ===

// 1. DMA 中断 (处理 ADC 数据搬运完成)
void DMA_IRQHandler(void) {
    // 1. 获取当前触发的中断索引 (IIDX)
    // 注意：只传 DMA 一个参数
    DL_DMA_EVENT_IIDX iidx = DL_DMA_getPendingInterrupt(DMA);
    // 2. 判断是否是通道 0 (ADC数据搬运)
    if (iidx == DL_DMA_EVENT_IIDX_DMACH0) {
        // 调用你的处理函数
        Fun_DMA_ADC_Handler();
        // 3. 清除中断
        // 注意：这里必须用 DL_DMA_INTERRUPT_CHANNEL0 (位掩码)，不能用 IIDX
        DL_DMA_clearInterruptStatus(DMA, DL_DMA_INTERRUPT_CHANNEL0);
    }
    
    // 如果有其他 DMA 通道（比如 UART TX），可以在这里继续判断
    // else if (iidx == DL_DMA_EVENT_IIDX_DMACH1) { ... }
}

// 2. 频率捕获中断 (TIMA0)
void CAPTURE_0_INST_IRQHandler(void) {
    // 检查 CC0 Down 事件 (根据 SysConfig 配置可能不同，通常是 Load/CC)
    // 这里使用通用的 Timer 中断检查
    if (DL_TimerA_getPendingInterrupt(CAPTURE_0_INST) == DL_TIMER_IIDX_CC0_DN) {
        Fun_Freq_Capture_Handler();
        DL_TimerA_clearInterruptStatus(CAPTURE_0_INST, DL_TIMER_IIDX_CC0_DN);
    }
}

// 3. UART 中断 (处理发送结束 EOT)
void UART_0_INST_IRQHandler(void) {
    if (DL_UART_Main_getPendingInterrupt(UART_0_INST) == DL_UART_MAIN_IIDX_EOT_DONE) {
        Proc_UART_Tx_Callback();
        DL_UART_Main_clearInterruptStatus(UART_0_INST, DL_UART_MAIN_IIDX_EOT_DONE);
    }
}