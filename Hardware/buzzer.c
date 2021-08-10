#include "buzzer.h"

#include "tim.h"

/**
 * @brief 初始化蜂鸣器，无需显式调用
 * 
 * @return int 
 */
int buzzer_init(void) {
    HAL_TIM_Base_Start(&htim12);
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
    return 0;
}
INIT_DEVICE_EXPORT(buzzer_init);

/**
 * @brief 设置蜂鸣器鸣叫
 * 
 * @param is_beep 是否鸣叫
 */
void buzzer_set_beep(int is_beep) {
    if (is_beep) {
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, 128);
    } else {
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, 0);
    }
}

/**
 * @brief 蜂鸣器鸣叫一定的时间
 * 
 * @param ms 指定的时间，单位：毫秒（ms）
 */
void buzzer_beep(rt_int32_t ms) {
    buzzer_set_beep(RT_TRUE);
    rt_thread_mdelay(ms);
    buzzer_set_beep(RT_FALSE);
}
