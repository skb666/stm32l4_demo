#ifndef __POWER_H__
#define __POWER_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t ts;
  uint32_t ss;
} TIMESTAMP;

time_t rtc_get_timestamp(void);
void rtc_get(TIMESTAMP *t);
uint32_t rtc_calc(TIMESTAMP *t_begin, TIMESTAMP *t_end);

void rtc_wake_init(void);
void enter_stop2(void);

#ifdef __cplusplus
}
#endif

#endif
