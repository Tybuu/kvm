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

static void emulation_task(void *pvParameters) {
  QueueHandle_t xReportQueue = (QueueHandle_t)pvParameters;
  hid_state_t state;
  memset(&state, 0, sizeof(hid_state_t));
  for (;;) {
    hid_command_t command;
    uart_packet_t packet = await_packet();
    // printf("[PICO] Packet Received (length: %d): [", packet.len);
    // for (int i = 0; i < packet.len; i++) {
    //   if (i < packet.len - 1) {
    //     printf("%x, ", packet.packet[i]);
    //   } else {
    //     printf("%x]\n", packet.packet[i]);
    //   }
    // }
    bool valid_command =
        deserialize_command(packet.packet, packet.len, &command);
    free_packet(&packet);
    if (valid_command) {
      hid_generic_report_t rep;
      if (generate_report(&state, &rep, command)) {
        xQueueSend(xReportQueue, &rep, 0);
      }
    }
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
        // printf("Report ID: %d | Report status: %s\n", rep.report_id,
        //        res ? "SUCCESS" : "FAILED");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

int main(void) {
  board_init();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  QueueHandle_t queue = xQueueCreate(16, sizeof(hid_generic_report_t));
  TaskHandle_t usb = NULL;
  TaskHandle_t uart = NULL;
  xTaskCreate(usb_task, "USB", 2048, (void *)queue, configMAX_PRIORITIES - 2,
              &usb);

  // xTaskCreate(blink_task, "Blink", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(emulation_task, "Emulation", 2048, (void *)queue,
              tskIDLE_PRIORITY + 2, &uart);

  start_uart_task(uart);
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
