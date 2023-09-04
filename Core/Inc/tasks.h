#ifndef __TASKS_H__
#define __TASKS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int8_t task_register(uint32_t tick, void (*run)(void), int32_t times, uint8_t adjustable);
void tasks_init(void);
void tasks_adjust(uint32_t offset);
void tasks_poll(void);

#ifdef __cplusplus
}
#endif

#endif
