#ifndef __CPUUSAGE_H
#define __CPUUSAGE_H

#include <rthw.h>
#include <rtthread.h>

/* 获取cpu利用率 */
void cpu_usage_init(void);
void cpu_usage_get(int argc, char *argv[]);

#endif
