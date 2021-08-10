#ifndef __GIMBAL_H
#define __GIMBAL_H

#include "main.h"

/* 机械零位角度 */
#define YAW_ZERO_DEGREE   2780
#define PITCH_ZERO_DEGREE 1500

#define MINIPC_BUFFER_SIZE 128

#define GIMBAL_EVENT_READY  0x01  // 云台初始化完成事件
#define GIMBAL_EVENT_MINIPC 0x02  // 收到 MINIPC 数据事件

/* 边界处理 */
#define BOUNDARY_PROCESS(var, ref, exp, max)  \
    if ((ref) - (exp) >= (max / 2)) {         \
        var = (ref)-max;                      \
    } else if ((ref) - (exp) <= -(max / 2)) { \
        var = (ref) + max;                    \
    } else {                                  \
        var = (ref);                          \
    }

typedef struct gm6020 {
    volatile uint16_t degree;      // 机械角度
    volatile int16_t  speed;       // 转速
    volatile int16_t  current;     // 实际转矩电流
    volatile uint8_t  temprature;  // 温度
} gm6020_t;

typedef struct m2006 {
    uint16_t degree;  // 机械角度
    int16_t  speed;   // 转速
    int16_t  torque;  // 转矩
} m2006_t;

typedef struct gimbal_data {
    volatile gm6020_t yaw;    // Yaw 轴电机
    volatile gm6020_t pitch;  // Pitch 轴电机
    volatile m2006_t  plunk;  // 拨弹轮电机
} gimbal_data_t;

typedef struct gimbal_behavior {
    volatile float   yaw_degree;    // Yaw 轴角度
    volatile int16_t yaw_speed;     // Yaw 轴速度
    volatile int16_t pitch_degree;  // Pitch 轴角度
    volatile int16_t pitch_speed;   // Pitch 轴速度
    volatile int16_t plunk_speed;   // 拨弹轮电机速度
} gimbal_behavior_t;

extern gimbal_behavior_t gimbal_behavior;
extern gimbal_data_t     gimbal_data;
extern rt_event_t        gimbal_event;
extern rt_event_t        gimbal_event;  // 云台事件集

int8_t gimbal_init(void);
void   gimbal_update_data(uint8_t id, uint8_t data[8]);
void   gimbal_set_voltage(int16_t yaw, int16_t pitch, int16_t plunk);
void   gimbal_handle(void);

#endif
