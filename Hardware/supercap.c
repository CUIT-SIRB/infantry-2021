#include "supercap.h"

#include "can.h"
#include "referee.h"

supercap_t supercap_data;
rt_timer_t supercap_timer;  // 超级电容定时器

static void supercap_timer_timeout(void *parameter);

/**
 * @brief 超级电容初始化，无需显式调用
 * 
 * @return int 
 */
int supercap_init(void) {
    /* 初始化超级电容定时器 */
    supercap_timer = rt_timer_create("supercap", supercap_timer_timeout, RT_NULL, 200,
                                     RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    /* 启动超级电容定时器 */
    rt_timer_start(supercap_timer);

    return 0;
}
INIT_DEVICE_EXPORT(supercap_init);

/**
 * @brief 更新超级电容数据
 *
 * @param data 原始数据
 */
void supercap_update_data(uint8_t Data[8]) {
    uint16_t *data_u16          = (uint16_t *)Data;
    supercap_data.input_voltage = data_u16[0] / 100.0F;
    supercap_data.cap_voltage   = data_u16[1] / 100.0F;
    supercap_data.input_current = data_u16[2] / 100.0F;
    supercap_data.target_power  = data_u16[3] / 100.0F;
}

/**
 * @brief 设置目标充电功率
 * 
 * @param target_power 
 */
void supercap_set_target_power(uint16_t target_power) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t             tx_data[8];
    uint32_t            tx_mailbox;

    /* 配置消息头 */
    tx_header.StdId              = 0x210U;
    tx_header.IDE                = CAN_ID_STD;
    tx_header.RTR                = CAN_RTR_DATA;
    tx_header.DLC                = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    /* 计算发送数据 */
    tx_data[0] = target_power >> 8;
    tx_data[1] = target_power;

    HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox);
}

/**
 * @brief 超级电容控制定时器超时函数
 * 
 * @param parameter 
 */
void supercap_timer_timeout(void *parameter) {
    /* 设置超级电容功率 */
    if (game_robot_status.chassis_power_limit != 80) {
        supercap_set_target_power(50 * 100);
    } else {
        supercap_set_target_power(80 * 100);
    }
}
