#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#include "Dri_CAN.h"
#include "usart.h"
#include <stdio.h>
#include "Bootloader.h"
#include <string.h>
#include "crc.h"

#define APP_UPDATE_CMD_0 'E'
#define APP_UPDATE_CMD_1 'A'
#define APP_UPDATE_CMD_2 'U'

typedef enum
{
    APP_UPDATE_WAIT_CMD = 0,
    APP_UPDATE_SEND_APP
} APP_UPDATE_STATE_E;

void App_update_Init(void);
void App_update_wait_cmd(void);
void App_update_send_app(void);
void App_update_work(void);

#endif /* __APP_UPDATE_H__ */
