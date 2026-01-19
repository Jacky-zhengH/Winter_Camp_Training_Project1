/* proc.h */
#ifndef PROC_H
#define PROC_H

#include "ti_msp_dl_config.h"

void Proc_Init(void);
void Proc_Task(void);
void Proc_Printf(const char *format, ...);
void Proc_UART_Tx_Callback(void); // EOT中断回调

#endif /* PROC_H */