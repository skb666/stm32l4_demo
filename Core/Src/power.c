#include "power.h"

#include "common.h"
#include "device.h"
#include "gpio.h"
#include "main.h"
#include "tasks.h"
#include "update.h"
#include "usart.h"

extern void SystemClock_Config(void);

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

void enter_gpio_config(void) {
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

void exit_gpio_config(void) {
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  uart_config();

  LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
}

void enter_stop2(void) {
  LL_IWDG_ReloadCounter(IWDG);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

  uart_wait_tx();
  LL_SYSTICK_DisableIT();
  tasks_update();
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
