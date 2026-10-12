#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bsp/board_api.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"
#include "device/usbd.h"
#include "emulate.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "report_types.h"
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
#include <string.h>

#define LED_PIN PICO_DEFAULT_LED_PIN
// #define BLINK_DELAY_MS 500
//
// static void blink_task(void *pvParameters) {
//   for (;;) {
//     gpio_put(LED_PIN, 1);
//     printf("[PICO] High!\n");
//     vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
//     gpio_put(LED_PIN, 0);
//     printf("[PICO] Low!\n");
//     vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY_MS));
//   }
// }

static void usb_task(void *pvParameters) {
  tusb_init();

  for (;;) {
    tud_task();
  }
}

bool send_report(uart_packet_t *rep) {
  return tud_hid_report(rep->data[0], &rep->data[2], rep->data[1]);
}

static void uart_task(void *pvParameters) {
  int count = 0;
  uart_packet_t *keyboard = NULL;
  uart_packet_t *mouse = NULL;
  for (;;) {
    uart_packet_t *packet = get_packet();
    if (packet != NULL) {
      if (packet->data[0] == REPORT_ID_MOUSE) {
        if (mouse == NULL) {
          mouse = packet;
        } else {
          mouse->data[2] = packet->data[2];
          int16_t a, b;
          memcpy(&a, &mouse->data[3], 2);
          memcpy(&b, &packet->data[3], 2);
          b += a;
          memcpy(&mouse->data[3], &b, 2);

          memcpy(&a, &mouse->data[5], 2);
          memcpy(&b, &packet->data[5], 2);
          b += a;
          memcpy(&mouse->data[5], &b, 2);

          memcpy(&a, &mouse->data[7], 2);
          memcpy(&b, &packet->data[7], 2);
          b += a;
          memcpy(&mouse->data[7], &b, 2);

          free_packet(packet);
        }
      } else if (packet->data[0] == REPORT_ID_NKRO) {
        if (keyboard != NULL) {
          free_packet(keyboard);
        }
        keyboard = packet;
      }
    }
    if (tud_hid_ready()) {
      if (keyboard != NULL && send_report(keyboard)) {
        free_packet(keyboard);
        keyboard = NULL;
      } else if (mouse != NULL && send_report(mouse)) {
        free_packet(mouse);
        mouse = NULL;
      }
    }
  }
}

int main(void) {
  board_init();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  TaskHandle_t usb = NULL;
  TaskHandle_t uart = NULL;
  xTaskCreate(usb_task, "USB", 2048, NULL, configMAX_PRIORITIES - 1, &usb);

  // xTaskCreate(blink_task, "Blink", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(uart_task, "Emulation", 2048, NULL, tskIDLE_PRIORITY + 2, &uart);

  start_uart_task();
  stdio_init_all();
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

  // echo back anything we received from host
  printf("Report: %x\n", buffer[0]);
  // tud_hid_report(0, buffer, bufsize);
}
