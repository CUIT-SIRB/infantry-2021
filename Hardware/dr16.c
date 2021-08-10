#include "dr16.h"

/* 遥控器原始数据 */
uint8_t dr16_data[32];
/* 遥控器事件集 */
rt_event_t dr16_event;
/* 遥控器控制数据 */
RC_CTL_TypeDef rc_ctl_data = {
    .rc =
        {
            .ch0 = 1024,
            .ch1 = 1024,
            .ch2 = 1024,
            .ch3 = 1024,
        },
};

/**
 * @brief 解码遥控器数据
 *
 */
void dr16_decode(void) {
    rc_ctl_data.rc.ch0 = ((int16_t)dr16_data[0] | ((int16_t)dr16_data[1] << 8)) & 0x07FF;
    rc_ctl_data.rc.ch1 = (((int16_t)dr16_data[1] >> 3) | ((int16_t)dr16_data[2] << 5)) & 0x07FF;
    rc_ctl_data.rc.ch2 =
        (((int16_t)dr16_data[2] >> 6) | ((int16_t)dr16_data[3] << 2) | ((int16_t)dr16_data[4] << 10)) & 0x07FF;
    rc_ctl_data.rc.ch3 = (((int16_t)dr16_data[4] >> 1) | ((int16_t)dr16_data[5] << 7)) & 0x07FF;

    rc_ctl_data.rc.s1         = ((dr16_data[5] >> 4) & 0x000C) >> 2;
    rc_ctl_data.rc.s2         = ((dr16_data[5] >> 4) & 0x0003);
    rc_ctl_data.mouse.x       = ((int16_t)dr16_data[6]) | ((int16_t)dr16_data[7] << 8);
    rc_ctl_data.mouse.y       = ((int16_t)dr16_data[8]) | ((int16_t)dr16_data[9] << 8);
    rc_ctl_data.mouse.z       = ((int16_t)dr16_data[10]) | ((int16_t)dr16_data[11] << 8);
    rc_ctl_data.mouse.press_l = dr16_data[12];
    rc_ctl_data.mouse.press_r = dr16_data[13];
    rc_ctl_data.key.prev_v    = rc_ctl_data.key.v;
    rc_ctl_data.key.v         = ((int16_t)dr16_data[14]);  // | ((int16_t)dr16_data[15] << 8);
}

/**
 * @brief 判断遥控器是否为初始化配置
 * 
 * @return int 是否为初始化配置
 */
int dr16_is_init_setting(void) {
    return rc_ctl_data.rc.s2 == 2 && rc_ctl_data.rc.s1 == 3;
}

/**
 * @brief 按键是否按下
 * 
 * @param key 要查询的按键，可用或运算判断多个按键
 * @return uint16_t 是否按下
 */
uint16_t dr16_is_pressed_down(uint16_t key) {
    return (rc_ctl_data.key.v & key) ? 1 : 0;
}

/**
 * @brief 按键是否点击（按下后弹起则有效）
 * 
 * @param key 要查询的按键
 * @return uint16_t 是否点击
 */
uint16_t dr16_is_clicked(uint16_t key) {
    return ((rc_ctl_data.key.prev_v & key) && !(rc_ctl_data.key.v & key)) ? 1 : 0;
}
