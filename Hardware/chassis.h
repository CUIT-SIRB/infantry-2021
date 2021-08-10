#ifndef __CHASSIS_H
#define __CHASSIS_H

#include "main.h"

typedef struct m3508 {
    volatile uint16_t degree;        // 机械角度
    volatile int16_t  speed;         // 速度
    volatile int16_t  expect_speed;  // 期望速度
    volatile int16_t  current;       // 电流
    volatile int16_t  set_current;   // 控制电流
    volatile uint8_t  temprature;    // 温度
} m3508_t;

typedef struct chassis_behavior {
    volatile int16_t x_speed;        // X速度
    volatile int16_t y_speed;        // Y 速度
    volatile int16_t angular_speed;  // 角速度
} chassis_behavior_t;

extern m3508_t            chassis_data[4];
extern chassis_behavior_t chassis_behavior;

void chassis_set_current(int16_t motor_1, int16_t motor_2, int16_t motor3, int16_t motor_4);
void chassis_update_data(uint8_t id, uint8_t data[8]);
void chassis_init(void);
void chassis_handler(int16_t speed_x, int16_t speed_y, int16_t speed_omega);

#endif
