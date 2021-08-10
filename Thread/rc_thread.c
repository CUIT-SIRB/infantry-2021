#include "rc_thread.h"

#include "dmalib.h"
#include "dr16.h"

extern uint8_t dr16_data[32];

/**
 * @brief 遥控线程入口，用于初始化遥控器以及离线检测
 *
 * @param parameter
 */
void rc_thread_entry(void *parameter) {
    rt_err_t ret;

    /* 初始化事件集 */
    dr16_event = rt_event_create("dr16", RT_IPC_FLAG_PRIO);  //! 需在启动串口接收前初始化
    /* 启动接收中断 */
    DMALIB_UART_Receive_IT(&DBUS_USART, dr16_data, DR16_RECEIVE_SIZE);

    while (1) {
        /* 等待遥控器在线事件 */
        ret = rt_event_recv(dr16_event, DR16_EVENT_ONLINE, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 50, RT_NULL);

        /* 遥控器断开连接 */
        if (ret != RT_EOK) {
            rc_ctl_data.rc.ch0 = 1024;
            rc_ctl_data.rc.ch1 = 1024;
            rc_ctl_data.rc.ch2 = 1024;
            rc_ctl_data.rc.ch3 = 1024;
            rc_ctl_data.rc.s1  = 3;
            rc_ctl_data.rc.s2  = 2;
        }
    }
}
