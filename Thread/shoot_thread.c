#include "shoot_thread.h"

#include <stdlib.h>

#include "chassis_thread.h"
#include "dr16.h"
#include "gimbal.h"
#include "referee_thread.h"
#include "shoot.h"

#define PLUNK_SPEED 1000  // 拨弹轮速度

/**
 * @brief 射击线程入口函数 *
 * @param parameter 参数指针
 */
void shoot_thread_entry(void *parameter) {
    shoot_init();
    /* 输出 PWM 信号 */
    shoot_set_duty(20);

    /* 等待云台初始化完成 */
    rt_event_recv(gimbal_event, GIMBAL_EVENT_READY, RT_EVENT_FLAG_AND, RT_WAITING_FOREVER, RT_NULL);

    while (1) {
        if (chassis_state == CHASSIS_STATE_RC_CTL) {
            switch (rc_ctl_data.rc.s2) {
                case 1:
                    /* 启动摩擦轮 */
                    if (shoot_behavior.friction_state == FRICTION_STATE_STOP) {
                        shoot_behavior.friction_state = FRICTION_STATE_SLOW_IN;
                    } else if (shoot_behavior.friction_state == FRICTION_STATE_READY) {
                        gimbal_behavior.plunk_speed = PLUNK_SPEED;
                    }
                    break;
                case 2:
                case 3:
                    /* 停止摩擦轮 */
                    if (shoot_behavior.friction_state == FRICTION_STATE_READY) {
                        shoot_behavior.friction_state = FRICTION_STATE_SLOW_OUT;
                        /* 停止拨弹轮 */
                        gimbal_behavior.plunk_speed = 0;
                    }
                    break;
            }
        }
        /* PC 控制模式 */
        else if (chassis_state == CHASSIS_STATE_PC_CTL) {
            /* 进入 PC 控制后马上开启摩擦轮，减少设射击延迟 */
            if (shoot_behavior.friction_state == FRICTION_STATE_STOP) {
                shoot_behavior.friction_state = FRICTION_STATE_SLOW_IN;
            }

            if (rc_ctl_data.mouse.press_l && cooling_heat <= COOLING_HEAT_MAX) {
                gimbal_behavior.plunk_speed = PLUNK_SPEED;
            } else {
                gimbal_behavior.plunk_speed = 0;
            }
        } else if (chassis_state == CHASSIS_STATE_STOP) {
            if (shoot_behavior.friction_state == FRICTION_STATE_READY) {
                shoot_behavior.friction_state = FRICTION_STATE_SLOW_OUT;
            }
        }
        rt_thread_mdelay(10);
    }
}
