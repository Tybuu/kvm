#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bsp/board_api.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"
#include "device/usbd.h"
#include "message_buffer.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "task.h"
#include "tusb.h"
#include "uart_task.h"
#include <hardware/address_mapped.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/regs/spi.h>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/uart.h>
#include <pico/stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define LED_PIN PICO_DEFAULT_LED_PIN
#define BLINK_DELAY_MS 500

static MessageBufferHandle_t xMessageBuffer;
static void blink_task(void *pvParameters) {
  for (;;) {
    gpio_put(LED_PIN, 1);
    printf("[PICO] High!\n");
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
    gpio_put(LED_PIN, 0);
    printf("[PICO] Low!\n");
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
  }
}

static void usb_task(void *pvParameters) {
  tusb_init();

  for (;;) {
    tud_task();
  }
}

#define BUFFER_SIZE 65
static void uart_task(void *pvParameters) {
  uint8_t buffer[BUFFER_SIZE + 3];
  buffer[0] = 0xA5;
  buffer[1] = 0x55;
  for (;;) {
    int len = xMessageBufferReceive(xMessageBuffer, buffer + 3, BUFFER_SIZE,
                                    portMAX_DELAY);
    if (len > 0) {
      buffer[2] = len;
      uart_send_bytes(buffer, len + 3);
    }
  }
}
int main(void) {
  board_init();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  xMessageBuffer = xMessageBufferCreate(65 * 32);

  TaskHandle_t usb = NULL;
  TaskHandle_t uart = NULL;

  xTaskCreate(usb_task, "USB", 2048, NULL, configMAX_PRIORITIES - 1, &usb);

  xTaskCreate(blink_task, "Blink", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(uart_task, "uart", 2048, NULL, tskIDLE_PRIORITY + 2, &uart);

  stdio_init_all();
  uart_init_task();
  // vTaskCoreAffinitySet(usb, 1 << 0);
  // vTaskCoreAffinitySet(blink, 1 << 0);
  // vTaskCoreAffinitySet(spi, 1 << 1);

  vTaskStartScheduler();

  for (;;) {
  }

  return 0;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // This example doesn't use multiple report and report ID
  xMessageBufferSend(xMessageBuffer, buffer, buffer[1] + 2, 0);
  // tud_hid_report(0, buffer, bufsize);
}
