#include "Flash.h"

/**
 * @brief 解锁Flash
 *
 */
void Flash_Unlock(void)
{
    HAL_FLASH_Unlock();
}

/**
 * @brief 锁定Flash
 *
 */
void Flash_Lock(void)
{
    HAL_FLASH_Lock();
}

/**
 * @brief 擦除单页
 *
 * @param page_addr 页起始地址(必须是页边界)
 * @return uint8_t 0:成功 1:失败
 */
uint8_t Flash_ErasePage(uint32_t page_addr)
{
    // 检查地址是否对齐到页边界
    if (page_addr % FLASH_PAGE_SIZE != 0)
    {
        return 1;
    }

    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Banks = FLASH_BANK_1;
    erase_init.PageAddress = page_addr;
    erase_init.NbPages = 1;
    uint32_t erase_error = 0;

    if (HAL_FLASHEx_Erase(&erase_init, &erase_error) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 擦除多页
 *
 * @param page_addr 起始页地址(必须是页边界)
 * @param pages 页数
 * @return uint8_t 0:成功 1:失败
 */
uint8_t Flash_ErasePages(uint32_t page_addr, uint16_t pages)
{
    // 检查地址是否对齐到页边界
    if (page_addr % FLASH_PAGE_SIZE != 0)
    {
        return 1;
    }

    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Banks = FLASH_BANK_1;
    erase_init.PageAddress = page_addr;
    erase_init.NbPages = pages;
    uint32_t erase_error = 0;

    if (HAL_FLASHEx_Erase(&erase_init, &erase_error) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 写入半字(16位)
 *
 * @param addr 写入地址(必须是2的倍数)
 * @param data 数据
 * @return uint8_t 0:成功 1:失败
 */
uint8_t Flash_WriteHalfWord(uint32_t addr, uint16_t data)
{
    // 检查地址对齐
    if (addr % 2 != 0)
    {
        return 1;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 写入数据
 *
 * @param addr 写入起始地址(必须是2的倍数)
 * @param data 数据指针
 * @param len 数据长度(必须是2的倍数)
 * @return uint8_t 0:成功 1:参数错误 2:写入失败
 */
uint8_t Flash_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
    // 参数检查
    if (addr % 2 != 0 || len % 2 != 0)
    {
        return 1; // 地址或长度未对齐
    }

    if (data == NULL || len == 0)
    {
        return 1; // 参数无效
    }

    // 按半字写入
    for (uint32_t i = 0; i < len; i += 2)
    {
        uint16_t halfword = data[i] | (data[i + 1] << 8);
        if (Flash_WriteHalfWord(addr + i, halfword) != 0)
        {
            return 2; // 写入失败
        }
    }

    return 0;
}

/**
 * @brief 读取数据
 *
 * @param addr 读取起始地址
 * @param buffer 接收缓冲区
 * @param len 读取长度
 * @return uint8_t 0:成功
 */
uint8_t Flash_Read(uint32_t addr, uint8_t *buffer, uint32_t len)
{
    if (buffer == NULL || len == 0)
    {
        return 1;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        buffer[i] = *(volatile uint8_t *)(addr + i);
    }
    return 0;
}

/**
 * @brief 检查区域是否已擦除(全为0xFF)
 *
 * @param addr 起始地址
 * @param len 长度
 * @return uint8_t 1:已擦除 0:未擦除
 */
uint8_t Flash_IsErased(uint32_t addr, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        if (*(volatile uint8_t *)(addr + i) != 0xFF)
        {
            return 0;
        }
    }
    return 1;
}
