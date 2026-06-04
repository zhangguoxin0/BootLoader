#include "BootLoader.h"
#include <string.h>

static uint8_t usart_rec_buff[BOOTLOADER_USART_REC_BUFF_LEN] = {0}; // 接收程序缓冲区
static uint16_t usart_rec_len = 0;                                  // 一次接收数据长度
static uint16_t usart_rec_fulllen = 0;                              // 接收数据总长度

static uint32_t flash_write_offest = 0; // 记录当前写入程序的偏移量

/**
 * @brief 串口开启中断接收之后，触发空闲帧时使用的回调函数
 *
 * @param huart
 * @param Size
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        // 保存实际接收到的数据长度
        usart_rec_len = Size;
        usart_rec_fulllen += usart_rec_len;
        
        // TODO:将接收到的数据写入到Flash中
        // 1.解锁Flash
        HAL_FLASH_Unlock();

        // 2.判断当前写入的地址是否为新的一页 => 需要擦除
        // 2.1.遍历需要写入的地址，长度为当前接收数据长度，如果全为0xFF则说明已经擦除过了
        uint8_t is_erase = 0;   // 是否需要擦除
        uint32_t page_addr = 0; // 当前页起始地址
        for (uint16_t i = 0; i < usart_rec_len; i++)
        {
            // 读取每一个位置的值
            uint8_t data = *(volatile uint8_t *)(APP_START_ADDR + i + flash_write_offest);
            if (data != 0xFF)
            {
                is_erase = 1;
                page_addr = (APP_START_ADDR + i + flash_write_offest) - ((APP_START_ADDR + i + flash_write_offest) % FLASH_PAGE_SIZE);
                break;
            }
        }
        // 2.2.如果需要擦除，则擦除当前页
        if (is_erase)
        {
            FLASH_EraseInitTypeDef erase_init;
            erase_init.TypeErase = FLASH_TYPEERASE_PAGES; // 按页擦除
            erase_init.Banks = FLASH_BANK_1;              // 擦除bank1的页
            erase_init.PageAddress = page_addr;           // 一般为当前页起始地址
            erase_init.NbPages = 1;                       // 擦除几页
            uint32_t erase_error = 0;
            HAL_FLASHEx_Erase(&erase_init, &erase_error);
        }

        // 3.重新加锁
        HAL_FLASH_Lock();

        // 使用结束后清空缓冲区，准备接收下一次数据
        memset(usart_rec_buff, 0, BOOTLOADER_USART_REC_BUFF_LEN);
        // 下一次接收
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, usart_rec_buff, BOOTLOADER_USART_REC_BUFF_LEN);
    }
}

/**
 * @brief 串口接收 => 准备接收A程序
 *
 */
void BootLoader_Init(void)
{
    // 清空初始化串口之前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    // 带中断的串口接收程序
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, usart_rec_buff, BOOTLOADER_USART_REC_BUFF_LEN);
}

/**
 * @brief 获取接收到的字节长度
 *
 * @param rec_len
 */
void BootLoader_GetRecLen(uint16_t *rec_len)
{
    *rec_len = usart_rec_fulllen;
}
