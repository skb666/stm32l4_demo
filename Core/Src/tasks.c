#include "tasks.h"

#include <stdint.h>
#include <stdio.h>

#include "key.h"
#include "main.h"
#include "update.h"

typedef enum {
  TASK_TICK_25MS,
  TASK_TICK_5S,
  TASK_TICK_MAX,
} TASK_TICK;

typedef struct {
  uint32_t refer;
  uint32_t tick;
  void (*run)(void);
} TASK;

static TASK task_list[TASK_TICK_MAX];

static KEY_VALUE getKey(void) {
  if (LL_GPIO_IsInputPinSet(KEY_GPIO_Port, KEY_Pin) == 1) {
    return K_PRESS;
  } else {
    return K_RELEASE;
  }
}

void task_25ms(void) {
  static KEY key = {
      .status = KS_RELEASE,
      .count = 0,
      .get = getKey,
  };
  static KEY_EVENT k_ev;

  k_ev = key_status_check(&key, 20);
  switch (k_ev) {
    case KE_PRESS: {
      printf("[KEY]: PRESS\n");
    } break;
    case KE_RELEASE: {
      printf("[KEY]: RELEASE\n");
      reboot_for_update();
    } break;
    case KE_LONG_PRESS: {
      printf("[KEY]: LONG_PRESS\n");
    } break;
    case KE_LONG_RELEASE: {
      printf("[KEY]: LONG_RELEASE\n");
    } break;
    default: {
    } break;
  }
}

void task_5s(void) {
  LL_IWDG_ReloadCounter(IWDG);
}

void tasks_init(void) {
  task_list[TASK_TICK_25MS] = (TASK){
      .refer = 25,
      .tick = HAL_GetTick(),
      .run = task_25ms,
  };
  task_list[TASK_TICK_5S] = (TASK){
      .refer = 5000,
      .tick = HAL_GetTick(),
      .run = task_5s,
  };
}

void tasks_poll(void) {
  for (int i = 0; i < TASK_TICK_MAX; ++i) {
    if (HAL_GetTick() - task_list[i].tick < task_list[i].refer) {
      continue;
    }
    task_list[i].tick = HAL_GetTick();

    task_list[i].run();
  }
}