#include "gimbal_thread.h"

#include <math.h>
#include <stdlib.h>

#include "bsp_imu.h"
#include "chassis_thread.h"
#include "dmalib.h"
#include "dr16.h"
#include "gimbal.h"
#include "pid.h"
#include "usart.h"

/* 装甲板自瞄 PID */
pid_type_t  autoaim_yaw_pid;
pid_type_t  autoaim_pitch_pid;
const float autoaim_pid_param[] = {160.0F, 5.0F, 150.0F};
/* 装甲板跟随状态 */
uint8_t gimbal_is_autoaim = 0;
/* 鼠标输入量滤波 */
static first_order_filter_type_t mouse_filter_x, mouse_filter_y;
/* Mini PC 串口缓冲区，四字节对齐以读取浮点数据 */
static uint8_t minipc_buffer[MINIPC_BUFFER_SIZE] ALIGN(4);

static void gimbal_pc_handler(void);

/**
 * @brief 云台线程入口
 *
 * @param parameter 参数指针
 */
void gimbal_thread_entry(void *parameter) {
    rt_tick_t prev_tick;

    /* 初始化装甲板自瞄 PID */
    pid_init(&autoaim_yaw_pid, PID_POSITION, autoaim_pid_param, 1000, 50);
    pid_init(&autoaim_pitch_pid, PID_POSITION, autoaim_pid_param, 1000, 50);

    /* 初始化一阶低通滤波 */
    first_order_filter_init(&mouse_filter_x, 0.01F, 0.03F);
    first_order_filter_init(&mouse_filter_y, 0.01F, 0.03F);

    // *云台初始化，耗时大约 3000ms
    prev_tick = rt_tick_get();
    if (gimbal_init()) {
        rt_kprintf("Gimbal init failed, thread exit.\n");
        return;
    }
    rt_kprintf("Gimbal init success, times: %d ms\n", rt_tick_get() - prev_tick);

    /* 初始化 Mini PC 串口 */
    DMALIB_UART_Receive_IT(&MINIPC_UART, minipc_buffer, MINIPC_BUFFER_SIZE);

    while (1) {
        /* 遥控器控制 */
        switch (chassis_state) {
            case CHASSIS_STATE_RC_CTL:
                /* 正常模式 */
                gimbal_behavior.pitch_degree += (rc_ctl_data.rc.ch1 - 1024) * 0.05F;
                gimbal_behavior.yaw_speed = (1024 - rc_ctl_data.rc.ch0) * 10.0F;
                break;
            case CHASSIS_STATE_PC_CTL:
                /* PC 控制模式 */
                gimbal_pc_handler();
                break;
            case CHASSIS_STATE_STOP:
                /* 停止模式 */
                gimbal_behavior.yaw_speed    = 0;
                gimbal_behavior.pitch_degree = 0;
                break;
        }

        rt_thread_mdelay(10);
    }
}

void gimbal_pc_handler(void) {
    rt_err_t      ret;
    float         autoaim_yaw;
    float         autoaim_pitch;
    minipc_data_t minipc_comm = {0};  // 通讯用结构体

    /* 鼠标输入量滤波 */
    first_order_filter_calc(&mouse_filter_x, rc_ctl_data.mouse.x);
    first_order_filter_calc(&mouse_filter_y, rc_ctl_data.mouse.y);

    /* 右键开启自瞄 */
    if (!gimbal_is_autoaim && rc_ctl_data.mouse.press_r) {
        pid_clear(&autoaim_yaw_pid);
        pid_clear(&autoaim_pitch_pid);
    }
    gimbal_is_autoaim = rc_ctl_data.mouse.press_r;

    /* Ctrl + Q 反转大符预测方向 */
    if (dr16_is_pressed_down(DR16_KEYBOARD_CTRL) && dr16_is_clicked(DR16_KEYBOARD_Q)) {
        minipc_comm.type = MINIPC_DATA_TYPE_TURN;
        HAL_UART_Transmit(&MINIPC_UART, (uint8_t *)&minipc_comm, sizeof(minipc_data_t), 0xFF);
    }
    /* 当风车速度最慢时按下 Ctrl 键发送 T0 */
    else if (dr16_is_clicked(DR16_KEYBOARD_CTRL)) {
        minipc_comm.type = MINIPC_DATA_TYPE_T0;
        HAL_UART_Transmit(&MINIPC_UART, (uint8_t *)&minipc_comm, sizeof(minipc_data_t), 0xFF);
    }

    if (gimbal_is_autoaim) {
        /* 开启自瞄 */
        ret = rt_event_recv(gimbal_event, GIMBAL_EVENT_MINIPC, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 50, RT_NULL);
        if (ret == RT_EOK) {
            autoaim_yaw   = *(float *)(minipc_buffer);
            autoaim_pitch = *(float *)(minipc_buffer + 4);

            if (!(isnan(autoaim_yaw) || isnan(autoaim_pitch))) {
                /* clang-format off */
                gimbal_behavior.yaw_speed   = 
                    pid_calc(&autoaim_yaw_pid, -autoaim_yaw, 0.0F) + mouse_filter_x.out * -10.0F;
                gimbal_behavior.pitch_speed = 
                    pid_calc(&autoaim_pitch_pid, -autoaim_pitch, 0.0F) + mouse_filter_y.out * -10.0F;
                /* clang-format on */
            }
        } else if (ret == -RT_ETIMEOUT) {
            gimbal_behavior.yaw_speed   = mouse_filter_x.out * -20.0F;
            gimbal_behavior.pitch_speed = mouse_filter_y.out * -20.0F;
        }
    } else {
        /* 鼠标手动瞄准 */
        gimbal_behavior.yaw_speed   = mouse_filter_x.out * -20.0F;
        gimbal_behavior.pitch_speed = mouse_filter_y.out * -20.0F;
    }
}
