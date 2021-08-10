#ifndef __REFEREE_THREAD_H
#define __REFEREE_THREAD_H

#include "main.h"

#define COOLING_HEAT_MAX 265

extern uint16_t cooling_heat;
extern float    chassis_power;

void referee_update_data(uint8_t data[4]);
void referee_thread_entry(void *parameter);

#endif
