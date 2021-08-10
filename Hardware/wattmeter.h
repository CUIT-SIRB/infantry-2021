#ifndef __WATTMETER_H
#define __WATTMETER_H

#include "main.h"

typedef struct wattmeter {
    float voltage;  //  电压
    float current;  // 电流
    float power;    // 功率
} wattmeter_t;

extern wattmeter_t wattmeter_data;

void wattmeter_update_data(uint8_t *data);

#endif
