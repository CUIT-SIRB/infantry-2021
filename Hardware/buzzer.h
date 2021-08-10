#if !defined(__BUZZER_H)
#define __BUZZER_H

#include "main.h"

int  buzzer_init(void);
void buzzer_set_beep(int is_beep);
void buzzer_beep(rt_int32_t ms);

#endif // __BUZZER_H
