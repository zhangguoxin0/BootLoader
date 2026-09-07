#include "App_Bootloader.h"

static uint8_t app_boot_update_status = BOOT_NO_UPDATE; // 是否需要更新

static uint8_t meta_app_buff[10] = {0}; // 元数据信息（4字节程序起始地址 + 4字节程序大小）
static uint32_t app_start_addr;         // A程序在W25Q32中的起始位置
static uint32_t app_size;               // W25Q32中A程序的大小

static uint8_t app_head_info[10] = {0}; // A程序头信息（4字节栈顶地址值 + 4字节复位中断）

static uint8_t flash_data_buff[FLASH_PAGE_SIZE + 1];

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
    app_start_addr = meta_app_buff[0] | meta_app_buff[1] << 8 | meta_app_buff[2] << 16 | meta_app_buff[3] << 24;
    app_size = meta_app_buff[4] | meta_app_buff[5] << 8 | meta_app_buff[6] << 16 | meta_app_buff[7] << 24;
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
    uint32_t app_reser_handle = app_head_info[4] | app_head_info[5] << 8 | app_head_info[6] << 16 | app_head_info[7] << 24;

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

/**
 * @brief 从W25Q32将程序写入到片上Flash的A程序区域
 *
 * @return uint8_t
 */
static uint8_t write_app_to_flash(void)
{
    uint8_t ret;
    // 剩余还没有写入的A程序大小
    uint32_t app_size_left;
    // 已经写入的A程序大小
    uint32_t writed_data_size;

    // 校验
    ret = check_mete_data();
    if (ret == 1)
    {
        return 1;
    }
    ret = check_app_info();
    if (ret == 1)
    {
        return 1;
    }

    // 擦除A区内容
    BootLoader_erase_flash();

    // 写入程序
    app_size_left = app_size;
    while (app_size_left > FLASH_PAGE_SIZE)
    {
        writed_data_size = app_size - app_size_left;
        // 从W25Q32读出1页内容
        W25Q32_Read(app_start_addr + writed_data_size, flash_data_buff, FLASH_PAGE_SIZE);
        app_size_left -= FLASH_PAGE_SIZE;
        // 写入1页内容到Flash
        Flash_Unlock();
        ret = Flash_Write(APP_START_ADDR + writed_data_size, flash_data_buff, FLASH_PAGE_SIZE);
        Flash_Lock();
        if (ret != 0)
        {
            return ret;
        }
    }
    // 写入最后一页
    if (app_size_left > 0)
    {
        writed_data_size = app_size - app_size_left;
        // 读取剩余内容
        W25Q32_Read(app_start_addr + writed_data_size, flash_data_buff, app_size_left);
        // 写入剩余内容到Flash（需要半字对齐）
        Flash_Unlock();
        uint32_t write_len = app_size_left;
        if (write_len % 2 != 0)
        {
            flash_data_buff[write_len] = 0xFF; // 补0xFF对齐到偶数字节
            write_len++;
        }
        ret = Flash_Write(APP_START_ADDR + writed_data_size, flash_data_buff, write_len);
        Flash_Lock();
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
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
        // 需要执行更新操作，将W25Q32中的程序写入到Flash中
        write_app_to_flash();
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
