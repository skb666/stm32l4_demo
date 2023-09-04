#include "power.h"

#include "common.h"
#include "device.h"
#include "gpio.h"
#include "main.h"
#include "tasks.h"
#include "update.h"
#include "usart.h"

extern void SystemClock_Config(void);

time_t rtc_get_timestamp(void) {
  static struct tm Time;

  memset(&Time, 0, sizeof(struct tm));

  LL_RTC_WaitForSynchro(RTC);

  Time.tm_year = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
  Time.tm_mon = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
  Time.tm_mday = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
  Time.tm_hour = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
  Time.tm_min = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
  Time.tm_sec = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));

  return mktime(&Time);
}

void rtc_get(TIMESTAMP *t) {
  t->ts = rtc_get_timestamp();
  t->ss = 0x7FF - LL_RTC_TIME_GetSubSecond(RTC);
}

uint32_t rtc_calc(TIMESTAMP *t_begin, TIMESTAMP *t_end) {
  uint32_t diff = 0;
  uint32_t tmp = 0;

  diff = (t_end->ts - t_begin->ts) * 1000;
  if (t_end->ss >= t_begin->ss) {
    tmp = (t_end->ss - t_begin->ss) * 1000 / 2048;
    diff += tmp;
  } else {
    tmp = (t_begin->ss - t_end->ss) * 1000 / 2048;
    if (diff > tmp) {
      diff -= tmp;
    }
  }

  return diff;
}

void rtc_wake_init(void) {
  LL_RTC_ClearFlag_WUT(RTC);
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
  LL_PWR_ClearFlag_WU();

  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_WAKEUP_Disable(RTC);

  while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1) {
  }

  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_16);
  LL_RTC_WAKEUP_SetAutoReload(RTC, 2048 * 5);

  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
  LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);

  LL_RTC_EnableIT_WUT(RTC);
  LL_RTC_WAKEUP_Enable(RTC);
  LL_RTC_EnableWriteProtection(RTC);
}

static void enter_gpio_config(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
  LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
}

static void exit_gpio_config(void) {
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  uart_config();

  LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
}

void enter_stop2(void) {
  LL_IWDG_ReloadCounter(IWDG);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

  LL_SYSTICK_DisableIT();
  uart_wait_tx();
  enter_gpio_config();

  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
  LL_LPM_EnableDeepSleep();
  __WFI();

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
  LL_PWR_ClearFlag_WU();
  LL_IWDG_ReloadCounter(IWDG);

  SystemClock_Config();
  exit_gpio_config();
  LL_SYSTICK_EnableIT();
}
