#include "Bootloader.h"
#include "Flash.h"
#include <string.h>

/**
 * @brief 跳转到指定程序
 *
 * @param app_addr 程序起始地址
 * @return uint8_t 0：跳转成功 1：跳转失败
 */
uint8_t BootLoader_jump_to_App(uint32_t app_addr)
{
    typedef void (*pFunc)(void);
    // 1.校验
    uint32_t app_stack_ptr = *(volatile uint32_t *)(app_addr);        // 栈顶地址
    uint32_t app_reser_handle = *(volatile uint32_t *)(app_addr + 4); // 复位中断地址
    // 校验栈顶地址的值
    if ((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        // 栈顶地址不合法
        printf("statck addr error\r\n");
        return 1;
    }
    // 校验复位中断地址
    if (app_reser_handle < app_addr || app_reser_handle > APP_END_ADDR)
    {
        // 复位中断地址不合法
        printf("reset handle error\r\n");
        return 1;
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
    SCB->VTOR = app_addr;
    // 3.跳转到程序复位中断
    pFunc jump_to_APP = (pFunc)app_reser_handle;
    jump_to_APP();

    return 0;
}

/**
 * @brief 擦除A区(App区域)全部内容
 * A区范围: 0x08008000 - 0x0807FFFF
 */
void BootLoader_erase_flash(void)
{
    uint32_t app_size = APP_END_ADDR - APP_START_ADDR + 1;
    uint16_t pages = app_size / FLASH_PAGE_SIZE;

    Flash_Unlock();
    Flash_ErasePages(APP_START_ADDR, pages);
    Flash_Lock();
}
