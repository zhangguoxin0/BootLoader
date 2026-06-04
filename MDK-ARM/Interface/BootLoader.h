#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "usart.h"


#define BOOTLOADER_USART_REC_BUFF_LEN 512 // BootLoader串口接收缓冲数组长度

// 程序写入的起始位置(A区起始位置),假设B区16K
#define APP_START_ADDR 0x08004000

void BootLoader_Init(void);
void BootLoader_GetRecLen(uint16_t *rec_len);

#endif /* __BOOTLOADER_H__ */
