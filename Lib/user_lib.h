#ifndef __USER_LIB_H
#define __USER_LIB_H

#include "stdint.h"

#define GET_BIT(reg, bit)   (((reg) >> bit) & 0x01)

typedef float fp32;

typedef struct {
    fp32 input;         //输入数据
    fp32 out;           //输出数据
    fp32 min_value;     //限幅最小值
    fp32 max_value;     //限幅最大值
    fp32 frame_period;  //时间间隔
} ramp_function_source_t;

typedef struct {
    fp32 input;         //输入数据
    fp32 out;           //滤波输出的数据
    fp32 num;           //滤波参数
    fp32 frame_period;  //滤波的时间间隔 单位 s
} first_order_filter_type_t;

void ramp_init(ramp_function_source_t *ramp_source_type);
void ramp_calc(ramp_function_source_t *ramp_source_type, fp32 input, fp32 step);
void first_order_filter_init(first_order_filter_type_t *first_order_filter_type, fp32 frame_period, const fp32 num);
void first_order_filter_calc(first_order_filter_type_t *first_order_filter_type, fp32 input);

#endif
