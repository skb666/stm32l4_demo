#ifndef __TASKS_H__
#define __TASKS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int8_t task_register(uint32_t tick, void (*run)(void));
void tasks_init(void);
void tasks_update(void);
void tasks_poll(void);

#ifdef __cplusplus
}
#endif

#endif
