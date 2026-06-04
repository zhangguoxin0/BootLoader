#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "usart.h"

#define BOOTLOADER_USART_REC_BUFF_LEN 512 // BootLoader串口接收缓冲数组长度

#define APP_START_ADDR 0x08004000 // 程序写入的起始位置(A区起始位置),假设B区16K
#define APP_END_ADDR 0x08007FFF   // A区终止位置
#define STACK_ADDR 0x20000000     // 栈顶地址

void BootLoader_receive_app(void);
void BootLoader_GetRecLen(uint16_t *rec_len);
uint8_t BootLoader_jump_to_App(void);
void BootLoader_erase_flash(uint32_t page_addr, uint16_t pages);

#endif /* __BOOTLOADER_H__ */
