#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void reset_global_irq(void);
void disable_global_irq(void);
void enable_global_irq(void);
void stack_backtrace(uint32_t pc, uint32_t lr, uint32_t sp);

#ifdef __cplusplus
}
#endif

#endif
