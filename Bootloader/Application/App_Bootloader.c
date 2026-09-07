#include "App_Bootloader.h"

static uint8_t app_boot_update_status = BOOT_NO_UPDATE; // 是否需要更新
static uint8_t meta_app_buff[10] = {0};                 // 元数据信息（4字节程序起始地址 + 4字节程序大小）
static uint8_t app_head_info[10] = {0};                 // A程序头信息（4字节栈顶地址值 + 4字节复位中断）

/**
 * @brief 校验元数据信息是否合法
 *
 * @return uint8_t 校验是否通过
 * @retval 0 校验通过
 * @retval 1 校验未通过
 */
static uint8_t check_mete_data(void)
{
    // 读取元数据信息（前4个字节为程序的起始地址，后4个字节为程序大小）
    W25Q32_Read(META_APP_ADDR, meta_app_buff, 8);
    uint32_t app_start_addr = meta_app_buff[0] | meta_app_buff[1] << 8 | meta_app_buff[2] << 16 | meta_app_buff[3] << 24;
    uint32_t app_size = meta_app_buff[5] | meta_app_buff[6] << 8 | meta_app_buff[7] << 16 | meta_app_buff[8] << 24;
    // 校验A程序在Flash中的存放位置是否合法
    if (app_start_addr < APP_START_ADDR)
    {
        printf("app start_addr error\n");
        return 1;
    }
    // 校验A程序大小是否合法
    if (app_size < APP_SIZE_MIN || app_size > APP_SIZE_MAX)
    {
        printf("app size errorn\n");
        return 1;
    }
    return 0;
}

/**
 * @brief 校验A程序头信息是否合法
 *
 * @return uint8_t 校验是否通过
 * @retval 0 校验通过
 * @retval 1 校验未通过
 */
static uint8_t check_app_info(void)
{
    // 读取A程序头信息
    W25Q32_Read(app_start_addr, app_head_info, 8);
    uint32_t app_stack_ptr = app_head_info[0] | app_head_info[1] << 8 | app_head_info[2] << 16 | app_head_info[3] << 24;
    uint32_t app_reser_handle = app_head_info[5] | app_head_info[6] << 8 | app_head_info[7] << 16 | app_head_info[8] << 24;

    // 校验栈顶地址的值是否合法
    if ((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        // 栈顶地址不合法
        printf("statck addr error\r\n");
        return 1;
    }
    // 校验复位中断地址是否合法
    if (app_reser_handle < APP_START_ADDR || app_reser_handle > APP_END_ADDR)
    {
        // 复位中断地址不合法
        printf("reset handle error\r\n");
        return 1;
    }
    return 0;
}

static void write_app_from_flash(void)
{
}

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
        // 需要恢复出厂设置，跳转前清除标志位
        uint8_t data[3] = {BOOT_NO_UPDATE, (uint8_t)(CHECK_KEY >> 8), (uint8_t)(CHECK_KEY & 0xFF)};
        W24C02_WriteStr(CHECK_UPDATE_ADDR, data, 3);
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
    if (GPIO_Pin == KEY1_Pin)
    {
        app_boot_update_status = BOOT_RESET;
    }
}
