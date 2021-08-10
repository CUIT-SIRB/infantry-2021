#if !defined(__SUPERCAP_H)
#define __SUPERCAP_H

#include "main.h"

/**
 * @brief 超级电容结构体
 *
 */
typedef struct supercap {
    volatile float input_voltage;  // 输入电压
    volatile float cap_voltage;    // 电容电压
    volatile float input_current;  // 输入电流
    volatile float target_power;   // 目标功率
} supercap_t;

extern supercap_t supercap_data;

void supercap_update_data(uint8_t Data[8]);
void supercap_set_target_power(uint16_t target_power);

#endif  // __SUPERCAP_H
