#include "user_lib.h"

/**
 * @brief          斜波函数初始化
 * @author         RM
 * @param[in]      斜波函数结构体
 * @param[in]      间隔的时间，单位 s
 * @param[in]      最大值
 * @param[in]      最小值
 * @retval         返回空
 */
void ramp_init(ramp_function_source_t *ramp_source_type) {
    ramp_source_type->input = 0.0f;
    ramp_source_type->out   = 0.0f;
}

/**
 * @brief          斜波函数计算，根据输入的值进行叠加， 输入单位为 /s 即一秒后增加输入的值
 * @author         RM
 * @param[in]      斜波函数结构体
 * @param[in]      输入值
 * @param[in]      增量步进值
 * @retval         返回空
 */
void ramp_calc(ramp_function_source_t *ramp_source_type, fp32 input, fp32 step) {
    ramp_source_type->input = input;
    if (ramp_source_type->out < ramp_source_type->input) {
        ramp_source_type->out += (ramp_source_type->input - ramp_source_type->out) < step
                                     ? (ramp_source_type->input - ramp_source_type->out)
                                     : step;
    } else {
        ramp_source_type->out -= (ramp_source_type->out - ramp_source_type->input) < step
                                     ? (ramp_source_type->out - ramp_source_type->input)
                                     : step;
    }
}

/**
 * @brief          一阶低通滤波初始化
 * @param[in]      一阶低通滤波结构体
 * @param[in]      间隔的时间，单位 s
 * @param[in]      滤波参数
 * @retval         返回空
 */
void first_order_filter_init(first_order_filter_type_t *first_order_filter_type, fp32 frame_period, const fp32 num) {
    first_order_filter_type->frame_period = frame_period;
    first_order_filter_type->num          = num;
    first_order_filter_type->input        = 0.0f;
    first_order_filter_type->out          = 0.0f;
}

/**
 * @brief          一阶低通滤波计算
 * @author         RM
 * @param[in]      一阶低通滤波结构体
 * @param[in]      间隔的时间，单位 s
 * @retval         返回空
 */
void first_order_filter_calc(first_order_filter_type_t *first_order_filter_type, fp32 input) {
    first_order_filter_type->input = input;
    first_order_filter_type->out =
        first_order_filter_type->num / (first_order_filter_type->num + first_order_filter_type->frame_period) *
            first_order_filter_type->out +
        first_order_filter_type->frame_period / (first_order_filter_type->num + first_order_filter_type->frame_period) *
            first_order_filter_type->input;
}
