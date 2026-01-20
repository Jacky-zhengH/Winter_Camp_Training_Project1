/* proc.h */
#ifndef PROC_H
#define PROC_H

#include "ti_msp_dl_config.h"
//------------------------
//  UART_RX     PB1
//  UART_TX     PB0
//------------------------
#define UART_SEND_BUFF_SIZE 100

extern volatile char uart_send_buff[UART_SEND_BUFF_SIZE];
extern volatile uint8_t uart_tx_complete_flag;  // 初始标志为已完成（允许新任务）
extern volatile uint8_t uart_tx_dma_complete_flag;  // 初始标志为已完成（允许新任务）
//---------------------------------------------------
void Proc_Init(void);
void uart_send_cmd(const char *format, ...);

// void Proc_Init(void);
// void Proc_Task(void);
// void Proc_Printf(const char *format, ...);
// void Proc_UART_Tx_Callback(void); // EOT中断回调

#endif /* PROC_H */