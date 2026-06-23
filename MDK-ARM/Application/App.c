#include "App.h"

static uint8_t app_boot_update_status = BOOT_NO_UPDATE; // 是否需要更新

/**
 * @brief 判断是否需要更新
 *
 */
void APP_bootloader_check_update(void)
{
    // 读取更新相关数据
    uint8_t data[3];
    W24C02_ReadStr(CHECK_UPDATE_ADDR, data, 3);
    // 校验密钥是否正确
    uint16_t key = data[1] << 8 | data[2];
    if (key != CHECK_KEY)
    {
        // 密钥不正确，不需要更新，重置密钥
        data[0] = BOOT_NO_UPDATE;
        data[1] = (uint8_t)(CHECK_KEY >> 8);
        data[2] = (uint8_t)(CHECK_KEY & 0xFF);
        W24C02_WriteStr(CHECK_UPDATE_ADDR, data, 3);
    }
    else
    {
        // 密钥正确，获取是否更新状态
        app_boot_update_status = data[0];
    }
}

/**
 * @brief 执行更新操作
 *
 */
void APP_bootloader_update(void)
{
    if (app_boot_update_status == BOOT_UPDATE)
    {
        // 需要执行更新操作，将W25Q32中的程序写入到flash中
        printf("update!\n");
    }
    else
    {
        // 不需要执行更新操作
        printf("no update\n");
    }
}

/**
 * @brief 执行跳转操作
 *
 */
void APP_bootloader_jump_app(void)
{
    // 无论更新与否，都需要在最后执行跳转操作到A程序
    BootLoader_jump_to_App();
}
