#include "referee_thread.h"

#include <stdlib.h>

#include "UI.h"
#include "can.h"
#include "chassis_thread.h"
#include "cmsis_compiler.h"
#include "dmalib.h"
#include "gimbal.h"
#include "referee.h"
#include "shoot.h"
#include "supercap.h"
#include "wattmeter.h"

uint16_t cooling_heat;   // 枪口热量数据
float    chassis_power;  // 底盘功率

/**
 * @brief 更新裁判系统数据
 *
 * @param data 原始数据
 */
void referee_update_data(uint8_t data[4]) {
    chassis_power         = data[0] + data[1] / 100.0F;
    cooling_heat          = (data[2] << 8) | data[3];
}

void referee_thread_entry(void *parameter) {
    // float watt;
    uint8_t is_show_ui = 0;  // 当前UI显示状态

    /* 等待云台初始化完成 */
    rt_event_recv(gimbal_event, GIMBAL_EVENT_READY, RT_EVENT_FLAG_AND, RT_WAITING_FOREVER, RT_NULL);
    rt_thread_mdelay(200);

    DMALIB_UART_Receive_IT(&REFEREE_UART, RefereeRecvBuf, REFEREE_RECV_BUF_SIZE);

    while (1) {
        // watt = wattmeter_data.Power;
        if ((chassis_state == CHASSIS_STATE_PC_CTL) != is_show_ui) {
            if (is_show_ui) {
                /* 清除所有自定义UI界面 */
                UI_DeleteAllGrapic();
            } else {
                /* 新增所有UI界面 */
                UI_Target(AddGrapic, 960, 445);
                UI_CapVoltage(AddGrapic, supercap_data.cap_voltage);
                UI_ChassisMode(AddGrapic, chassis_spinning);
                UI_Stuck(AddGrapic, shoot_is_stuck);
            }
            is_show_ui = chassis_state == CHASSIS_STATE_PC_CTL;
        }
        UI_CapVoltage(ModifyGrapic, supercap_data.cap_voltage);
        UI_ChassisMode(ModifyGrapic, chassis_spinning);
        UI_Stuck(ModifyGrapic, shoot_is_stuck);
        rt_thread_mdelay(50);
    }
}
