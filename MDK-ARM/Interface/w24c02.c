#include "w24c02.h"

void W24C02_Init(void)
{
    MX_I2C2_Init();
}

/**
 * @brief 读取一个字节
 *
 * @param byte_addr 读取地址
 * @return uint8_t 读取到的数据
 */
uint8_t W24C02_ReadByte(uint8_t byte_addr)
{
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c2, W24C02_ADDR_R, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
    return data;
}

/**
 * @brief 写入一个字节
 *
 * @param byte_addr 写入地址
 * @param data 写入的字节
 */
void W24C02_WriteByte(uint8_t byte_addr, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_W, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

/**
 * @brief 读取多个字节
 *
 * @param byte_addr 读取起始地址
 * @param data 读取到的数据
 * @param len 读取到数据的长度
 */
void W24C02_ReadStr(uint8_t byte_addr, uint8_t *data, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c2, W24C02_ADDR_R, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

/**
 * @brief 写入多个字节
 *
 * @param byte_addr 写入字节起始地址
 * @param data 写入的数据
 * @param len 写入数据的长度
 */
void W24C02_WriteStr(uint8_t byte_addr, uint8_t *data, uint16_t len)
{
    uint8_t start_page_addr = byte_addr % W24C02_PAGE_SIZE; // 写入地址在首页的位置
    uint8_t start_page_remain_len = 16 - start_page_addr;   // 首页剩余地址
    uint8_t data_remain_len;                                // 还需要写入的数据长度
    uint8_t page_remain_num;                                // 需要写入的整页数
    uint8_t tail_len;                                       // 最后一页数据长度

    if ((W24C02_SIZE - byte_addr) < len)
    {
        // 空间不足
        return;
    }
    else
    {
        // 需要写入的数据长度小于首页剩余空间
        if (len <= start_page_remain_len)
        {
            // 直接写入
            HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_W, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
            HAL_Delay(5);
            return;
        }
        // 需要写入的数据长度大于首页剩余空间
        else
        {

            // 先将首页写满
            HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_W, byte_addr, I2C_MEMADD_SIZE_8BIT, data, start_page_remain_len, 1000);
            HAL_Delay(5);

            data_remain_len = len - start_page_remain_len;        // 还需要写入的数据长度
            page_remain_num = data_remain_len / W24C02_PAGE_SIZE; // 需要写入的整页数
            tail_len = data_remain_len % W24C02_PAGE_SIZE;        // 最后一页数据长度
        }
        // 写入剩余中间数据
        for (uint8_t i = 0; i < page_remain_num; i++)
        {
            HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_W, byte_addr + start_page_remain_len + (i * W24C02_PAGE_SIZE), I2C_MEMADD_SIZE_8BIT, data + start_page_remain_len + (i * W24C02_PAGE_SIZE), W24C02_PAGE_SIZE, 1000);
            HAL_Delay(5);
        }
        // 写入末页数据
        if (tail_len)
        {
            HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_W, byte_addr + start_page_remain_len + (page_remain_num * W24C02_PAGE_SIZE), I2C_MEMADD_SIZE_8BIT, data + start_page_remain_len + (page_remain_num * W24C02_PAGE_SIZE), tail_len, 1000);
            HAL_Delay(5);
        }
    }
}
