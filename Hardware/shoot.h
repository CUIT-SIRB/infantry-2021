#ifndef __SHOOT_H
#define __SHOOT_H

#include "main.h"

/**
 * @brief 摩擦轮状态
 *
 */
typedef enum {
    FRICTION_STATE_STOP = 0,  // 停止射击
    FRICTION_STATE_SLOW_IN,   // 加速
    FRICTION_STATE_SLOW_OUT,  // 减速
    FRICTION_STATE_READY,     // 可以射击
} friction_state_t;

typedef struct shoot_behavior {
    volatile uint16_t friction_duty;  // 摩擦轮占空比，范围：[16, 800]
    friction_state_t  friction_state;
} shoot_behavior_t;

extern shoot_behavior_t shoot_behavior;
extern uint8_t          shoot_is_stuck;

void shoot_init(void);
void shoot_set_duty(uint16_t Duty);
void shoot_handle(void);

#endif
