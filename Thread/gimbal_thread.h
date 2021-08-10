#ifndef __GIMBAL_THREAD_H
#define __GIMBAL_THREAD_H

#include "main.h"

typedef enum minipc_data_type {
    MINIPC_DATA_TYPE_T0 = 0,
    MINIPC_DATA_TYPE_GIMBAL_INFO,
    MINIPC_DATA_TYPE_TURN
} minipc_data_type_t;

typedef struct minipc_data {
    minipc_data_type_t type;
    float              pitch_angle;
    int                pitch_speed;
    float              yaw_angle;
    int                yaw_speed;
} minipc_data_t;

void gimbal_thread_entry(void *parameter);

#endif
