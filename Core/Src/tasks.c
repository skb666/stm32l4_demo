#include "tasks.h"

#include <stdio.h>

#include "key.h"
#include "main.h"
#include "power.h"
#include "ring_fifo.h"
#include "update.h"

#define TASKS_LIST_MAX 32

typedef enum {
  TASK_TICK_25MS = 25,
  TASK_TICK_1S = 1000,
  TASK_TICK_5S = 5000,
  TASK_TICK_10S = 10000,
} TASK_TICK;

typedef struct {
  uint8_t adjustable;
  uint32_t refer;
  uint32_t tick;
  int32_t times;
  void (*run)(void);
} TASK;

typedef struct {
  uint32_t tick_run;
  RING_FIFO *ring;
} TASKS;

ring_def_static(TASK, tasks_ring, TASKS_LIST_MAX, 0);
static TASKS tasks;
static uint32_t g_tick;
static TIMESTAMP g_ts;

static KEY_VALUE getKey(void) {
  if (LL_GPIO_IsInputPinSet(KEY_GPIO_Port, KEY_Pin) == 1) {
    return K_PRESS;
  } else {
    return K_RELEASE;
  }
}

static void task_25ms(void) {
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
      Error_Handler();
    } break;
    case KE_LONG_PRESS: {
      printf("[KEY]: LONG_PRESS\n");
    } break;
    case KE_LONG_RELEASE: {
      printf("[KEY]: LONG_RELEASE\n");
      reboot_for_update();
    } break;
    default: {
    } break;
  }
}

static void task_1s(void) {
  TIMESTAMP ts;
  uint32_t offset = 0;

  //enter_stop2();

  rtc_get(&ts);
  //uart_printf("%u %u %u %u\n", g_ts.ts, g_ts.ss, ts.ts, ts.ss);

  offset = rtc_calc(&g_ts, &ts);
  memcpy(&g_ts, &ts, sizeof(TIMESTAMP));
  if (offset > tasks.tick_run) {
    tasks_adjust(offset - tasks.tick_run);
    //uart_printf("adjust: %u\n", offset - tasks.tick_run);
  }

  LL_IWDG_ReloadCounter(IWDG);
}

static void task_5s(void) {
  uart_printf("5s\n");
}

static void task_10s(void) {
  uart_printf("10s\n");
}

int8_t task_register(uint32_t tick, void (*run)(void), int32_t times, uint8_t adjustable) {
  TASK task;

  if (ring_size(tasks.ring) >= TASKS_LIST_MAX) {
    return -1;
  }

  task.refer = tick;
  task.run = run;
  task.times = times;
  task.adjustable = adjustable;
  task.tick = tick;
  if (ring_push(tasks.ring, &task)) {
    Error_Handler();
  }

  return 0;
}

void tasks_init(void) {
  g_tick = HAL_GetTick();
  rtc_get(&g_ts);

  memset(&tasks, 0, sizeof(TASKS));
  tasks.ring = &tasks_ring;
  tasks.tick_run = 1000;

  task_register(TASK_TICK_25MS, task_25ms, -1, 0);
  task_register(TASK_TICK_1S, task_1s, -1, 0);
  task_register(TASK_TICK_5S, task_5s, 6, 1);
  task_register(TASK_TICK_10S, task_10s, 3, 1);
}

void tasks_adjust(uint32_t offset) {
  int i, task_num = 0;
  TASK task;

  task_num = ring_size(tasks.ring);

  for (i = 0; i < task_num; ++i) {
    if (ring_pop(tasks.ring, &task)) {
      Error_Handler();
    }
    if (task.adjustable) {
      if (task.tick > offset) {
        task.tick -= offset;
      } else {
        task.tick = 0;
      }
    }
    if (ring_push(tasks.ring, &task)) {
      Error_Handler();
    }
  }
}

void tasks_poll(void) {
  uint32_t tick_diff = 0;
  int i, task_num = 0;
  TASK task;

  tick_diff = HAL_GetTick() - g_tick;
  if (tick_diff < 1) {
    return;
  }

  g_tick = HAL_GetTick();
  task_num = ring_size(tasks.ring);

  for (i = 0; i < task_num; ++i) {
    if (ring_pop(tasks.ring, &task)) {
      Error_Handler();
    }

    if (task.tick > 0) {
      task.tick -= tick_diff;
      if (ring_push(tasks.ring, &task)) {
        Error_Handler();
      }
      continue;
    }

    task.tick = task.refer;
    task.run();
    if (task.times > 0) {
      task.times -= 1;
    }

    if (task.times) {
      if (ring_push(tasks.ring, &task)) {
        Error_Handler();
      }
    }
  }
}
