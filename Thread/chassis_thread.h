#ifndef __CHASSIS_THREAD_H
#define __CHASSIS_THREAD_H

#include "main.h"

/**
 * @brief 云台状态定义
 *
 */
typedef enum {
    CHASSIS_STATE_STOP = 0,  // 停止模式
    CHASSIS_STATE_RC_CTL,    // 遥控器控制模式
    CHASSIS_STATE_PC_CTL,    // PC 控制模式
} chassis_state_t;

extern chassis_state_t chassis_state;
extern uint8_t         chassis_spinning;

void chassis_thread_entry(void *parameter);

#endif
