#include "BootLoader.h"
#include <string.h>

static uint8_t usart_rec_buff[BOOTLOADER_USART_REC_BUFF_LEN] = {0}; // 接收程序缓冲区
static uint16_t usart_rec_len = 0;                                  // 一次接收数据长度
static uint16_t usart_rec_fulllen = 0;                              // 接收数据总长度

static uint32_t flash_write_offest = 0; // 记录当前写入程序的偏移量

static uint8_t last_byte = 0;      // 一次接收的末尾字节
static uint8_t last_byte_flag = 0; // 是否有末尾字节的存在

/**
 * @brief Flash擦除：判断当前写入的地址是否为新的一页 => 需要擦除
 *
 */
static void flash_erase(void)
{
    // 1.遍历需要写入的地址，长度为当前接收数据长度，如果全为0xFF则说明已经擦除过了
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
    // 2.如果需要擦除，则擦除当前页
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
}

/**
 * @brief 上次写入有遗留字节处理方法
 *
 */
static void flash_with_last(void)
{
    // 拼接字节
    for (uint16_t i = 0; i < usart_rec_len; i += 2)
    {
        uint32_t flash_addr = APP_START_ADDR + i + flash_write_offest; // 需要写入的位置
        uint16_t data16;
        // 特殊处理第一个字节
        if (i == 0)
        {
            // 与上一个遗留字节拼接
            data16 = last_byte | (usart_rec_buff[i] << 8);
        }
        else
        {
            data16 = usart_rec_buff[i - 1] | (usart_rec_buff[i] << 8);
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_addr, data16); // 在指定位置写入半字
    }
}
/**
 * @brief 上次写入无遗留字节处理方法
 *
 */
static void flash_without_last(void)
{
    for (uint16_t i = 0; i < usart_rec_len; i += 2)
    {
        uint32_t flash_addr = APP_START_ADDR + i + flash_write_offest; // 需要写入的位置
        uint16_t data16;                                               // 拼接字节
        if (i + 1 < usart_rec_len)
        {
            data16 = usart_rec_buff[i] | (usart_rec_buff[i + 1] << 8);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_addr, data16); // 在指定位置写入半字
        }
    }
}

/**
 * @brief Flash写入：使用16位写入
 *
 */
static void flash_write_halfword(void)
{
    // 当前能够写入的字节为偶数(偶+偶 奇+奇 情况)
    if ((usart_rec_len + last_byte_flag) % 2 == 0)
    {
        // 奇+奇：上一次的遗留字节需要作为本次第一个字节写入
        if (last_byte_flag)
        {
            flash_with_last();
            flash_write_offest += usart_rec_len + 1; // 更新偏移量
        }
        // 偶+偶：直接写入本次接收的内容
        else
        {
            flash_without_last();
            flash_write_offest += usart_rec_len; // 更新偏移量
        }
        last_byte_flag = 0;
    }
    // 本次写入还回遗留一个字节(奇+偶 偶+奇 情况)
    else
    {
        // 奇+偶：上一次的遗留字节需要作为本次第一个字节写入，且回遗留最后一个字节到下一次
        if (last_byte_flag)
        {
            flash_with_last();
            flash_write_offest += usart_rec_len; // 更新偏移量
        }
        // 偶+奇：本次接收到的最后一个字节回遗留到下一次
        else
        {
            flash_without_last();
            flash_write_offest += usart_rec_len - 1; // 更新偏移量
        }
        last_byte = usart_rec_buff[usart_rec_len - 1];
        last_byte_flag = 1;
    }
}

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

        // 2.Flash擦除
        flash_erase();

        // 3.Flash写入：使用16位写入
        flash_write_halfword();

        // 4.重新加锁
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

/**
 * @brief 跳转到A程序
 *
 */
void BootLoader_jump_to_App(void)
{
    typedef void (*pFunc)(void);
    // 1.校验
    uint32_t app_stack_ptr = *(volatile uint32_t *)(APP_START_ADDR);        // 栈顶地址
    uint32_t app_reser_handle = *(volatile uint32_t *)(APP_START_ADDR + 4); // 复位中断地址
    // 校验栈顶地址的值
    if((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        // 栈顶地址不合法
        return;
    }
    // 校验复位中断地址
    if(app_reser_handle < APP_START_ADDR || app_reser_handle > APP_END_ADDR)
    {
        // 复位中断地址不合法
        return;
    }
    // 2.注销BootLoader程序
    // 2.1.关闭中断
    __disable_irq();

    NVIC_DisableIRQ(EXTI9_5_IRQn);
    NVIC_DisableIRQ(USART1_IRQn);
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    HAL_DeInit();

    // 2.2.设置堆栈指针
    __set_MSP(app_stack_ptr);
    // 2.3.重定向中断向量表
    SCB->VTOR = APP_START_ADDR;
    // 3..跳转到A程序复位中断
    pFunc jump_to_APP = (pFunc)app_reser_handle;
    jump_to_APP();
}
