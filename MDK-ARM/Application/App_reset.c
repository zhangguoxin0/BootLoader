#include "App_reset.h"
#include <string.h>
#include <stdlib.h>

uint8_t app_rec_start_buff[64] = {0};
uint16_t app_rec_start_len = 0;
uint32_t app_rec_total_len = 0;

App_status app_status = APP_STATUS_INIT; // 记录当前应用层状态

extern uint32_t last_rec_time;     // 记录当前一次接收数据的时间
extern uint16_t usart_rec_fulllen; // 接收数据总长度

/**
 * @brief 初始化reset => 打印日志启动
 *
 */
void App_Init(void)
{
    printf("reset start\r\n");
    printf("wait user send data\r\n");
    printf("send 'start:len' to start\r\n");
    app_status = APP_STATUS_INIT;
}

/**
 * @brief 等待用户传输确认
 *
 */
void App_Run(void)
{
    // 使用非中断的方式接收 => 区分接收程序

    // 挂起等待接收 一直等待接收buff满或者收到idle空闲帧
    HAL_UARTEx_ReceiveToIdle(&huart1, app_rec_start_buff, 64, &app_rec_start_len, 0xFFFFFF);
    if (app_rec_start_buff > 0)
    {
        // 判断数据中是否包含start:len
        char *start_str = strstr((char *)app_rec_start_buff, "start:");
        if (start_str != NULL)
        {
            // 保存len的值
            app_rec_total_len = atoi((char *)start_str + 6);
            if (app_rec_total_len > 0)
            {
                printf("app len:%d\r\n", app_rec_total_len);
                // 修改状态到下一个阶段
                app_status = APP_STATUS_RUN;
            }
            else
            {
                printf("len error\r\n");
            }
        }
        else
        {
            printf("data error\r\n");
        }
    }
}

/**
 * @brief 接收数据
 *
 */
void App_RecData(void)
{
    // 接收完成之后 修改状态为check_data
    // 软件方式:等待10s
    if ((HAL_GetTick() - last_rec_time > 10000) && (last_rec_time != 0))
    {
        // 已经10s中没有接收到数据
        app_status = APP_STATUS_CHECK_DATA;
    }
}

/**
 * @brief 校验数据
 *
 * @return uint8_t 0：校验通过 1：校验未通过
 */
uint8_t App_CheckData(void)
{
    if (usart_rec_fulllen == app_rec_total_len)
    {
        // 说明长度一致，没问题
        printf("app rec ok\r\n");
        app_status = APP_STATUS_JUMP_APP;
        return 0;
    }
    else
    {
        printf("app rec error or timeout\r\n");
        return 1;
    }
}

/**
 * @brief 跳转程序
 *
 * @return uint8_t 0：跳转失败 1：跳转成功
 */
uint8_t App_Jump2App(void)
{
    printf("jump to app\r\n");
    uint8_t ret = BootLoader_jump_to_App();
    return ret;
}

void App_Work(void)
{
    switch (app_status)
    {
    case APP_STATUS_INIT:
        App_Run();
        break;
    case APP_STATUS_RUN:
        // 接收数据准备工作
        // 确认要写入flash => 提前擦除Flash页 直接先擦除10页20K
        BootLoader_erase_flash(APP_START_ADDR, 10);
        printf("flash erase ok\r\n");
        printf("ready to receive appr\r\n");
        app_status = APP_STATUS_REC_DATA;
        BootLoader_receive_app();
        break;
    case APP_STATUS_REC_DATA:
        App_RecData();
        break;
    case APP_STATUS_CHECK_DATA:
        // 检查数据情况
        if (App_CheckData())
        {
            printf("app rec error,system reset\r\n");
            NVIC_SystemReset();
        }
        break;
    case APP_STATUS_JUMP_APP:
        if (App_Jump2App())
        {
            // 跳转失败，重新启动bootloader
            printf("jump app error,system reset\r\n");
            NVIC_SystemReset();
        }
        break;
    default:
        break;
    }
}
