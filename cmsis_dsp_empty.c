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
extern volatile uint16_t gADCBuffer[FFT_LENGTH];


int main(void)
{
    int i = 0;
    SYSCFG_DL_init();
    VCA810_Init();
    Proc_Init();
    Fun_Init();
    //------------------------------------------
    /*设置挡位*/
    VCA810_SetGain(VCA_GAIN_LoW);
    float gain=VCA810_GetGainFactor();
    uart_send_cmd("system gain: %.2f\r\n",gain);
    LED_Debug(2, 200); 
    LED_Debug(3, 200);
    while (1) 
    {
        __WFI();
        //uart_send_cmd("system count: %d\r\n",i++);
        // DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_LED1_PIN);
        // delay_cycles(500*ms_cycle); // 假设80MHz，这大概是0.5秒
    }
}

//----------------------------------------------------------
// === Interrupt Handlers ===
void ADC12_0_INST_IRQHandler(){
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
        case DL_ADC12_IIDX_DMA_DONE:
            DL_TimerG_stopCounter(TIMER_SAMPLE_INST);
            //中断处理
            //proccess
            __BKPT();
            Fun_Start_Sampling();
            break;
        default:
            break;
    }
}


// UART 中断 (处理发送结束 EOT)
void UART0_IRQHandler(){
    volatile uint32_t res = DL_UART_Main_getPendingInterrupt(UART_0_INST);
    switch (res) {
        // case  DL_UART_IIDX_TX:{
        //    uart_send(uart_send_buff);
        // }break;
        case DL_UART_IIDX_EOT_DONE:{
            uart_tx_complete_flag = 1;
        }break;
        case DL_UART_MAIN_IIDX_DMA_DONE_TX:
            uart_tx_dma_complete_flag = 1;
        break;
    }
}