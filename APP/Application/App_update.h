#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#include <usart.h>
#include "Dri_CAN.h"
#include "LED.h"
#include "crc.h"

#define APP_UPDATE_CMD "EAU"

#define APP_DATA_MAX_LEN 16384

typedef enum
{
    UPDATE_IDLE = 0,
    UPDATE_RECV_SEND_CMD,
    UPDATE_RECV_DATA,
    UPDATE_RECV_CHECK_DATA,
} Update_State_t;

void App_update_Init(void);
void App_update_send_update_cmd(void);
void App_update_receive_app_data(void);
void App_update_change_boot_mode(void);
void App_update_work(void);

#endif /* __APP_UPDATE_H__ */
