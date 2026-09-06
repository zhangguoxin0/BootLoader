#include "App_Bootloader.h"

static uint8_t app_boot_update_status = BOOT_NO_UPDATE; // 是否需要更新

/**
 * @brief 判断是否需要更新
 *
 */
void APP_bootloader_check_update(void)
{
    printf("bootloader start\n");
    printf("check update\n");
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
 * @brief 检查是否需要恢复出厂设置
 *
 */
void APP_bootloader_check_default(void)
{
    HAL_Delay(3000);
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
        // TODO:将W25Q32的程序写入到flash中
        printf("update!\n");
    }
    else if (app_boot_update_status == BOOT_NO_UPDATE)
    {
        // 不需要执行更新操作
        printf("no update\n");
    }
    else if (app_boot_update_status == BOOT_RESET)
    {
        printf("reset\n");
    }
}

/**
 * @brief 执行跳转操作
 *
 */
void APP_bootloader_jump_app(void)
{
    if (app_boot_update_status == BOOT_RESET)
    {
        // 需要恢复出厂设置
        // 跳转到出厂设置的默认程序 地址0x08004000
        BootLoader_jump_to_App(RESET_START_ADDR);
    }
    else
    {
        // 不需要恢复出厂设置
        // 无论更新与否，都需要在最后执行跳转操作到A程序 地址0x08008000
        BootLoader_jump_to_App(APP_START_ADDR);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        app_boot_update_status = BOOT_RESET;
    }
}
