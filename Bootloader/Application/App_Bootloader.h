#ifndef __APP_BOOTLOADER_H__
#define __APP_BOOTLOADER_H__

#include "usart.h"
#include "Bootloader.h"
#include "gpio.h"
#include "W24C02.h"
#include "W25Q32.h"
#include "Flash.h"

// 是否需要更新状态
#define CHECK_UPDATE_ADDR 0x10 // 存储更新状态的位置
#define BOOT_UPDATE 0x01       // 需要更新
#define BOOT_NO_UPDATE 0x02    // 不需要更新
#define BOOT_RESET 0x03        // 需要恢复出厂设置

// 校验密钥
#define CHECK_KEY_ADDR 0x11 // 存储校验密钥的位置
#define CHECK_KEY 0x486A    // 密钥值

// 元数据信息地址
#define META_APP_ADDR 0x00000000

// 程序存储判断条件
#define APP_START_ADDR_MIN 0x001000
#define APP_SIZE_MIN 500
#define APP_SIZE_MAX 0x78000

void APP_bootloader_check_update(void);
void APP_bootloader_check_default(void);
void APP_bootloader_update(void);
void APP_bootloader_jump_app(void);

#endif /* __APP_BOOTLOADER_H__ */
