#ifndef __APP_H__
#define __APP_H__

#include "BootLoader.h"
#include "usart.h"

typedef enum
{
    APP_STATUS_INIT,
    APP_STATUS_RUN,
    APP_STATUS_REC_DATA,
    APP_STATUS_CHECK_DATA,
    APP_STATUS_JUMP_APP
}
App_status;

void App_Init(void);
void App_Run(void);
void App_RecData(void);
uint8_t App_CheckData(void);
uint8_t App_Jump2App(void);
void App_Work(void);

#endif /* __APP_H__ */
