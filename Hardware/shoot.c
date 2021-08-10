#include "shoot.h"

#include "gimbal.h"
#include "pid.h"
#include "tim.h"
#include "user_lib.h"

#define FRICTION_MIN_DUTY     450
#define FRICTION_MAX_DUTY     575
#define FRICTION_SPEEDUP_STEP 0.2F

shoot_behavior_t shoot_behavior = {
    .friction_duty = FRICTION_MIN_DUTY,
};

static rt_timer_t             shoot_timer;                                    // 发射机构定时器
static ramp_function_source_t ramp_friction;                                  // 摩擦轮斜坡函数
static pid_type_t             pid_plunk_speed;                                // 拨弹轮 PID
const fp32                    pid_plunk_speed_parm[3] = {2.0F, 0.05F, 0.0F};  // 拨弹轮 PID 参数

uint8_t shoot_is_stuck = 0;  // 是否卡弹
int16_t plunk_voltage;       // 拨弹轮电压，在 gimbal_handle 中发送

static void shoot_timer_timeout(void *parameter);

/**
 * @brief 发射机构初始化
 *
 */
void shoot_init(void) {
    /* 初始化 PWM */
    HAL_TIM_Base_Start(&FRICTION_TIM);
    HAL_TIM_PWM_Start(&FRICTION_TIM, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&FRICTION_TIM, TIM_CHANNEL_3);

    /* 初始化摩擦轮斜坡函数 */
    ramp_init(&ramp_friction);
    ramp_friction.out = FRICTION_MIN_DUTY;

    /* 拨弹轮 PID 初始化 */
    pid_init(&pid_plunk_speed, PID_POSITION, pid_plunk_speed_parm, 10000, 10000);

    /* clang-format off */
    /* 初始化发射机构定时器 */
    shoot_timer = rt_timer_create("shoot",
                                  shoot_timer_timeout,
                                  RT_NULL,
                                  1,
                                  RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_HARD_TIMER);
    rt_timer_start(shoot_timer);
    /* clang-format on */

    HAL_GPIO_WritePin(LASER_GPIO_Port, LASER_Pin, GPIO_PIN_SET);
}

/**
 * @brief 发射机构控制定时器超时函数
 * 
 * @param parameter 
 */
void shoot_timer_timeout(void *parameter) {
    shoot_handle();
}

/**
 * @brief 设置摩擦轮占空比
 *
 * @param Duty 占空比，范围：[16, 800]
 */
void shoot_set_duty(uint16_t Duty) {
    __HAL_TIM_SET_COMPARE(&FRICTION_TIM, TIM_CHANNEL_2, Duty);
    __HAL_TIM_SET_COMPARE(&FRICTION_TIM, TIM_CHANNEL_3, Duty);
}

/**
 * @brief 发射机构周期控制函数
 *
 */
void shoot_handle(void) {
    static uint16_t plunk_flag = 0;

    /* 摩擦轮状态机 */
    switch (shoot_behavior.friction_state) {
        case FRICTION_STATE_SLOW_IN:
            /* 开启充能装置 */
            HAL_GPIO_WritePin(POWERCTL_4_GPIO_Port, POWERCTL_4_Pin, GPIO_PIN_SET);
            /* 加速 */
            ramp_calc(&ramp_friction, FRICTION_MAX_DUTY, FRICTION_SPEEDUP_STEP);
            shoot_behavior.friction_duty = ramp_friction.out;
            /* 状态切换 */
            if (shoot_behavior.friction_duty == FRICTION_MAX_DUTY) {
                shoot_behavior.friction_state = FRICTION_STATE_READY;
            }
            break;
        case FRICTION_STATE_SLOW_OUT:
            /* 减速 */
            ramp_calc(&ramp_friction, FRICTION_MIN_DUTY, FRICTION_SPEEDUP_STEP);
            shoot_behavior.friction_duty = ramp_friction.out;
            /* 状态切换 */
            if (shoot_behavior.friction_duty == FRICTION_MIN_DUTY) {
                shoot_behavior.friction_state = FRICTION_STATE_STOP;
            }
            break;
        case FRICTION_STATE_READY:
            /* 启动拨弹轮 */
            break;
        case FRICTION_STATE_STOP:
            /* 关闭充能装置 */
            HAL_GPIO_WritePin(POWERCTL_4_GPIO_Port, POWERCTL_4_Pin, GPIO_PIN_RESET);
            break;
    }
    /* 设置摩擦轮占空比 */
    shoot_set_duty(shoot_behavior.friction_duty);

    /* 计算拨弹轮速度 PID */
    if (gimbal_data.plunk.torque > 9000 && gimbal_behavior.plunk_speed > 0)
        plunk_flag = 300;
    if (plunk_flag != 0) {
        plunk_flag--;
        shoot_is_stuck              = 1;
        gimbal_behavior.plunk_speed = -1000;
    } else {
        shoot_is_stuck = 0;
    }

    /* 设置拨弹轮电压 */
    plunk_voltage = pid_calc(&pid_plunk_speed, gimbal_data.plunk.speed, gimbal_behavior.plunk_speed);
}
