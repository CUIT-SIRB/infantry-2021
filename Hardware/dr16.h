#ifndef __DR16_H
#define __DR16_H

#include "main.h"

#define DR16_RECEIVE_SIZE 32  // DMA 接收长度

#define DR16_EVENT_ONLINE 0x01  // 遥控器在线事件

#define DR16_KEYBOARD_W     (0x0001)
#define DR16_KEYBOARD_S     (0x0002)
#define DR16_KEYBOARD_A     (0x0004)
#define DR16_KEYBOARD_D     (0x0008)
#define DR16_KEYBOARD_SHIFT (0x0010)
#define DR16_KEYBOARD_CTRL  (0x0020)
#define DR16_KEYBOARD_Q     (0x0040)
#define DR16_KEYBOARD_E     (0x0080)

/**
 * @brief 接收机数据结构体
 *
 */
typedef struct __RC_CTL_TypeDef {
    /* 遥控器数据 */
    struct __RC_TypeDef {
        uint16_t ch0;
        uint16_t ch1;
        uint16_t ch2;
        uint16_t ch3;
        uint8_t  s1;
        uint8_t  s2;
    } rc;
    /* 鼠标数据 */
    struct __RC_Mouse_TypeDef {
        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t press_l;
        uint8_t press_r;
    } mouse;
    /* 键盘数据 */
    struct __RC_Key_TypeDef {
        /**
         * bit0: W
         * bit1: S
         * bit2: A
         * bit3: D
         * bit4: Shift
         * bit5: Ctrl
         * bit6: Q
         * bit7: E
         */
        uint16_t v;
        uint16_t prev_v;
    } key;
} RC_CTL_TypeDef;

extern RC_CTL_TypeDef rc_ctl_data;
extern rt_event_t     dr16_event;

void            dr16_decode(void);
int             dr16_is_init_setting(void);
inline uint16_t dr16_is_pressed_down(uint16_t key);
inline uint16_t dr16_is_clicked(uint16_t key);

#endif
