#ifndef __APP_H__
#define __APP_H__

#include "w24c02.h"
#include "usart.h"
#include "BootLoader.h"

// 是否需要更新状态
#define CHECK_UPDATE_ADDR 0x10 // 存储更新状态的位置
#define BOOT_UPDATE 0x01       // 需要更新
#define BOOT_NO_UPDATE 0x02    // 不需要更新

// 校验密钥
#define CHECK_KEY_ADDR 0x11 // 存储校验密钥的位置
#define CHECK_KEY 0x486A    // 密钥值

void APP_bootloader_check_update(void);
void APP_bootloader_update(void);
void APP_bootloader_jump_app(void);

#endif /* __APP_H__ */
