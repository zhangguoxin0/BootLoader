#include "App_update.h"

static APP_UPDATE_STATE_E g_app_update_state = APP_UPDATE_WAIT_CMD; // 更新程序工作状态机状态

static Dri_CAN_Message app_update_CAN_message_rx; // 更新程序CAN消息接收缓冲区结构体
static Dri_CAN_Message app_update_CAN_message_tx; // 更新程序CAN消息接收缓冲区结构体

static uint16_t update_date_total_len;  // 记录程序的总长度
static uint16_t update_date_sended_len; // 已经发送的数据长度

/**
 * @brief 初始化上位机更新程序
 *
 */
void App_update_Init(void)
{
    printf("App update init\r\n");
    g_app_update_state = APP_UPDATE_WAIT_CMD;
    Dri_CAN_Init();
    printf("App udate wait cmd...\r\n");
}

/**
 * @brief 等待下位机发送更新请求
 *
 */
void App_update_wait_cmd(void)
{
    uint8_t ret = Dri_CAN_Receive(&app_update_CAN_message_rx);
    if (ret == 1)
    {
        if (app_update_CAN_message_rx.DLC == 3)
        {
            if (app_update_CAN_message_rx.data[0] == APP_UPDATE_CMD_0 && app_update_CAN_message_rx.data[1] == APP_UPDATE_CMD_1 && app_update_CAN_message_rx.data[2] == APP_UPDATE_CMD_2)
            {
                g_app_update_state = APP_UPDATE_SEND_APP;
                printf("App udate send app\r\n");
            }
            else
            {
                printf("Cmd is error\r\n");
            }
        }
        else
        {
            printf("Cmd len is invalid\r\n");
        }
    }
}

/**
 * @brief 向下位机发送更新程序
 *
 */
void App_update_send_app(void)
{
    uint8_t buffer[8] = {0};
    uint8_t len = 0;

    if (update_date_sended_len < update_date_total_len)
    {

        if (update_date_total_len - update_date_sended_len >= 8)
        {
            len = 8;
        }
        else
        {
            len = update_date_total_len - update_date_sended_len;
        }

        for (uint_8_t i = 0; i < len; i++)
        {
            buffer[i] = *(volatile uint8_t *)(APP_START_ADDR + update_date_len + i);
        }

        app_update_CAN_message_tx.StdId = 0x1;
        app_update_CAN_message_tx.IDE = CAN_ID_STD;
        app_update_CAN_message_tx.RTR = CAN_RTR_DATA;
        app_update_CAN_message_tx.DLC = len;
        memcpy(app_update_CAN_message_tx.data, buffer, len);
        Dri_CAN_Send(&app_update_CAN_message_tx);

        update_date_sended_len += len;
        if (update_date_sended_len % 256 == 0)
        {
            HAL_Delay(100);
        }
    }
    else
    {
        g_app_update_state = APP_UPDATE_WAIT_CMD;
    }
}

/**
 * @brief 更新程序工作状态机
 *
 */
void App_update_work(void)
{
    switch (g_app_update_state)
    {
    case APP_UPDATE_WAIT_CMD:
        App_update_wait_cmd();
        break;
    case APP_UPDATE_SEND_APP:
        App_update_send_app();
        break;
    default:
        break;
    }
}
