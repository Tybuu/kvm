#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bsp/board_api.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"
#include "device/usbd.h"
#include "projdefs.h"
#include "queue.h"
#include "task.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <hardware/gpio.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/uart.h>
#include <pico/stdio.h>
#include <stdio.h>

#define LED_PIN PICO_DEFAULT_LED_PIN
#define BLINK_DELAY_MS 500

static void blink_task(void *pvParameters) {
  QueueHandle_t xQueue = (QueueHandle_t)pvParameters;

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  hid_generic_report_t rep;
  rep.report_id = REPORT_ID_MOUSE;
  rep.len = sizeof(hid_mouse_report_t);
  mouse_clear(&rep.payload.mouse);
  for (;;) {
    gpio_put(LED_PIN, 1);
    printf("Move X by 10!\n");
    mouse_move(&rep.payload.mouse, 10, 0);
    xQueueSend(xQueue, &rep, 0);
    mouse_clear_delta(&rep.payload.mouse);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
    gpio_put(LED_PIN, 0);
    printf("Move Y by 10!\n");
    mouse_move(&rep.payload.mouse, 0, 10);
    xQueueSend(xQueue, &rep, 0);
    mouse_clear_delta(&rep.payload.mouse);
    vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
  }
}

static void usb_task(void *pvParameters) {
  QueueHandle_t xQueue = (QueueHandle_t)pvParameters;
  hid_generic_report_t rep;
  tusb_init();

  for (;;) {
    tud_task();
    if (tud_hid_ready()) {
      if (xQueueReceive(xQueue, &rep, 0) == pdTRUE) {
        bool res = tud_hid_report(rep.report_id, &rep.payload, rep.len);
        printf("Report ID: %d | Report status: %s\n", rep.report_id,
               res ? "SUCCESS" : "FAILED");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void print_hex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02X ", data[i]);
  }
  printf("\n");
}

int main(void) {
  board_init();
  // uart_init(uart0, 115200);
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);
  stdio_init_all();

  QueueHandle_t queue = xQueueCreate(16, sizeof(hid_generic_report_t));
  xTaskCreate(usb_task, "USB", 2048, (void *)queue, configMAX_PRIORITIES - 2,
              NULL);

  xTaskCreate(blink_task, "Blink", 2048, (void *)queue, tskIDLE_PRIORITY + 1,
              NULL);
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

  // echo back anything we received from host
  printf("Report: %x\n", buffer[0]);
  // tud_hid_report(0, buffer, bufsize);
}
