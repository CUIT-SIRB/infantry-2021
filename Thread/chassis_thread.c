#include "chassis_thread.h"

#include <math.h>

#include "chassis.h"
#include "dr16.h"
#include "gimbal.h"
#include "gimbal_thread.h"
#include "pid.h"
#include "supercap.h"

#define RC_RAMP_STEP 1.0F  // 斜坡步进值

ramp_function_source_t rc_ctl_ramp[3];  // 输入量斜坡函数结构体

pid_type_t pid_follow_gimbal;  // 跟随云台 PID
const fp32 pid_follow_gimbal_param[3] = {0.10F, 0.0F, 0.0F};

chassis_state_t chassis_state;         // 底盘状态
uint8_t         chassis_spinning = 0;  // 小陀螺

/**
 * @brief 底盘线程入口，用于控制底盘电机运动
 *
 * @param parameter 指向参数的指针
 */
void chassis_thread_entry(void *parameter) {
    rt_tick_t prev_tick;
    int16_t   pc_speed_x, pc_speed_y;
    float     angular_speed;

    prev_tick = rt_tick_get();

    /* 初始化底盘 */
    chassis_init();
    /* 初始化跟随云台 PID */
    pid_init(&pid_follow_gimbal, PID_POSITION, pid_follow_gimbal_param, 300, 300);
    /* 初始化遥控器斜坡函数 */
    for (uint8_t i = 0; i < 3; i++) {
        ramp_init(rc_ctl_ramp + i);
        rc_ctl_ramp[i].out = 1024;
    }

    rt_kprintf("Chassis init success, times: %d ms\n", rt_tick_get() - prev_tick);

    /* 等待云台初始化完成 */
    rt_event_recv(gimbal_event, GIMBAL_EVENT_READY, RT_EVENT_FLAG_AND, RT_WAITING_FOREVER, RT_NULL);

    while (1) {
        /* 优先判断停止模式 */
        if (rc_ctl_data.rc.s2 == 2) {
            if (chassis_state != CHASSIS_STATE_STOP) {
                chassis_state = CHASSIS_STATE_STOP;
            }
        } else {
            switch (rc_ctl_data.rc.s1) {
                case 1:
                    /* 小陀螺模式 */
                    chassis_spinning = 1;
                    break;
                case 3:
                    /* 正常模式 */
                    if (chassis_state != CHASSIS_STATE_RC_CTL) {
                        pid_clear(&pid_follow_gimbal);
                    }
                    chassis_state    = CHASSIS_STATE_RC_CTL;
                    chassis_spinning = 0;
                    break;
                case 2:
                    /* PC 控制模式 */
                    chassis_state = CHASSIS_STATE_PC_CTL;
                    /* 小陀螺模式，E 键弹起时触发 */
                    if (dr16_is_clicked(DR16_KEYBOARD_E)) {
                        chassis_spinning = !chassis_spinning;
                    }
                    break;
            }
        }

        /* PC 控制输入量 */
        /* Shift 键加速 */
        if (dr16_is_pressed_down(DR16_KEYBOARD_SHIFT)) {
            pc_speed_y = (dr16_is_pressed_down(DR16_KEYBOARD_W) - dr16_is_pressed_down(DR16_KEYBOARD_S)) * 350;
            pc_speed_x = (dr16_is_pressed_down(DR16_KEYBOARD_A) - dr16_is_pressed_down(DR16_KEYBOARD_D)) * 350;
        } else {
            pc_speed_y = (dr16_is_pressed_down(DR16_KEYBOARD_W) - dr16_is_pressed_down(DR16_KEYBOARD_S)) * 150;
            pc_speed_x = (dr16_is_pressed_down(DR16_KEYBOARD_A) - dr16_is_pressed_down(DR16_KEYBOARD_D)) * 150;
        }

        /**
         * *斜坡函数处理速度
         * *角速度不需要做斜坡处理，防止不易控制转向
         */
        if (chassis_state == CHASSIS_STATE_PC_CTL) {
            ramp_calc(rc_ctl_ramp, pc_speed_x + 1024, RC_RAMP_STEP);
            ramp_calc(rc_ctl_ramp + 1, pc_speed_y + 1024, RC_RAMP_STEP);
        } else {
            ramp_calc(rc_ctl_ramp, 2048 - rc_ctl_data.rc.ch2, RC_RAMP_STEP);
            ramp_calc(rc_ctl_ramp + 1, rc_ctl_data.rc.ch3, RC_RAMP_STEP);
        }

        int16_t mechdeg;
        if (chassis_state == CHASSIS_STATE_RC_CTL) {
            /* 优弧劣弧处理 */
            BOUNDARY_PROCESS(mechdeg, gimbal_data.yaw.degree, YAW_ZERO_DEGREE, 8192);
            angular_speed = -pid_calc(&pid_follow_gimbal, mechdeg, YAW_ZERO_DEGREE);
        } else if (chassis_state == CHASSIS_STATE_PC_CTL) {
            /* 死区 */
            if (gimbal_data.yaw.degree - YAW_ZERO_DEGREE >= 150) {
                angular_speed = -pid_calc(&pid_follow_gimbal, gimbal_data.yaw.degree - 200, YAW_ZERO_DEGREE);
            } else if (YAW_ZERO_DEGREE - gimbal_data.yaw.degree >= 150) {
                angular_speed = -pid_calc(&pid_follow_gimbal, gimbal_data.yaw.degree + 200, YAW_ZERO_DEGREE);
            } else {
                angular_speed = 0;
            }
        }

        chassis_behavior.x_speed = rc_ctl_ramp[0].out - 1024;
        chassis_behavior.y_speed = rc_ctl_ramp[1].out - 1024;
        /* 小陀螺模式 */
        if (chassis_spinning) {
            chassis_behavior.angular_speed = 150;
        } else {
            chassis_behavior.angular_speed = angular_speed;
        }

        rt_thread_mdelay(2);
    }
}
