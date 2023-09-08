#include "common.h"

#include <stdio.h>

#include "main.h"

static uint32_t s_irq_count = 0;

void reset_global_irq(void) {
  s_irq_count = 0;
}

void disable_global_irq(void) {
  __disable_irq();
  s_irq_count++;
  return;
}

void enable_global_irq(void) {
  if (s_irq_count != 0) {
    s_irq_count--;
  }

  if (0 == s_irq_count) {
    __enable_irq();
  }
  return;
}

void stack_backtrace(uint32_t pc, uint32_t lr, uint32_t sp) {
  uint32_t cnt = 0;

  printf("pc: %08lx, lr: %08lx, sp: %08lx\n", pc, lr, sp);

  printf("%d: %08lx\n", cnt++, pc);
  if ((lr & 0xfff00000) == 0x08000000) {
    printf("%d: %08lx\n", cnt++, lr - 4);
  }
  for (uint32_t *addr = (uint32_t *)sp; (uint32_t)addr < 0x2000c000; addr++) {
    if ((*addr & 0xfff00000) == 0x08000000) {
      printf("%d: %08lx\n", cnt++, *addr - 4);
    }
  }
}

