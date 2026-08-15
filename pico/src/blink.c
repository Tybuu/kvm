#include "FreeRTOS.h"
#include "pico/stdlib.h"
#include "task.h"
#include "tusb.h"
#include <stdio.h>

#define LED_PIN PICO_DEFAULT_LED_PIN
#define BLINK_DELAY_MS 500

static void blink_task(void *pvParameters) {
  (void)pvParameters;

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  for (;;) {
    gpio_put(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
    gpio_put(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
  }
}

int main(void) {
  tusb_init();
  stdio_init_all();
  xTaskCreate(blink_task, "Blink", configMINIMAL_STACK_SIZE, NULL,
              tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  for (;;) {
  }

  return 0;
}
