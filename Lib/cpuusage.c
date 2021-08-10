#include "cpuusage.h"
#include <rthw.h>
#include <rtthread.h>

#define CPU_USAGE_CALC_TICK 1000
#define CPU_USAGE_LOOP      100

static rt_uint8_t  cpu_usage_major = 0, cpu_usage_minor = 0;
static rt_uint32_t total_count = 0;

/**
 * @brief 计算 CPU 使用率的空闲钩子
 *
 */
static void cpu_usage_idle_hook() {
    rt_tick_t            tick;
    rt_uint32_t          count;
    volatile rt_uint32_t loop;

    if (total_count == 0) {
        /* get total count */
        rt_enter_critical();
        tick = rt_tick_get();
        while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK) {
            total_count++;
            loop = 0;
            while (loop < CPU_USAGE_LOOP)
                loop++;
        }
        rt_exit_critical();
    }

    count = 0;
    /* get CPU usage */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK) {
        count++;
        loop = 0;
        while (loop < CPU_USAGE_LOOP)
            loop++;
    }

    /* calculate major and minor */
    if (count < total_count) {
        count           = total_count - count;
        cpu_usage_major = (count * 100) / total_count;
        cpu_usage_minor = ((count * 100) % total_count) * 100 / total_count;
    } else {
        total_count = count;

        /* no CPU usage */
        cpu_usage_major = 0;
        cpu_usage_minor = 0;
    }
}

/**
 * @brief 获取 CPU 使用率
 *
 * @param major CPU 使用率的整数部分
 * @param minor CPU 使用率的小数部分
 */
void cpu_usage_get(int argc, char *argv[]) {
    /* 打印 CPU 使用率 */
    rt_kprintf("cpu usage: %d.%d%%\n", cpu_usage_major, cpu_usage_minor);
}
MSH_CMD_EXPORT(cpu_usage_get, get cpu usage);

/**
 * @brief 初始化 CPU 使用率计算
 *
 */
void cpu_usage_init() {
    /* 设置空闲线程钩子函数 */
    rt_thread_idle_sethook(cpu_usage_idle_hook);
}
INIT_COMPONENT_EXPORT(cpu_usage_init);
