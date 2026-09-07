#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "usart.h"

#define BOOTLOADER_USART_REC_BUFF_LEN 512 // BootLoader串口接收缓冲数组长度

#define APP_START_ADDR 0x08008000   // 程序写入的起始位置(A区起始位置)
#define APP_END_ADDR 0x0807FFFF     // A区终止位置(512KB Flash结束地址)
#define RESET_START_ADDR 0x08004000 // 恢复出厂设置程序起始位置
#define RESET_END_ADDR 0x08007FFF   // 恢复出厂设置程序终止位置
#define STACK_ADDR 0x20000000       // 栈顶地址

uint8_t BootLoader_jump_to_App(uint32_t app_addr);
void BootLoader_erase_flash(void);

#endif /* __BOOTLOADER_H__ */
