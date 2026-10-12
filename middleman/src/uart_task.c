#include "uart_task.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "task.h"
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/dreq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/uart.h>
#include <stdint.h>
#include <stdio.h>
#define UART_ID uart1
#define BAUD_RATE 3000000
#define TX_PIN 4
#define RX_PIN 5

volatile static int dma_tx_chan;

static TaskHandle_t task;

void on_uart_tx_dma() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (dma_channel_get_irq0_status(dma_tx_chan)) {
    dma_channel_acknowledge_irq0(dma_tx_chan);
    if (task != NULL) {
      vTaskNotifyGiveFromISR(task, &xHigherPriorityTaskWoken);
      task = NULL;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void uart_init_task() {
  // Initialize global state
  // Initialize UART
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(RX_PIN, GPIO_FUNC_UART);

  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
  uart_set_fifo_enabled(UART_ID, true);

  // Enable DMA
  uint dreq_uart_tx = (UART_ID == uart0) ? DREQ_UART0_TX : DREQ_UART1_TX;
  dma_tx_chan = dma_claim_unused_channel(true);
  dma_channel_config_t c = dma_channel_get_default_config(dma_tx_chan);

  // Only increment on writes as we're writing to RAM
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);

  // Enable DMA interrupts
  channel_config_set_dreq(&c, dreq_uart_tx);
  irq_set_exclusive_handler(DMA_IRQ_0, on_uart_tx_dma);
  irq_set_enabled(DMA_IRQ_0, true);
  dma_channel_set_irq0_enabled(dma_tx_chan, true);

  // Start the DMA task
  dma_channel_configure(dma_tx_chan, &c, &uart_get_hw(UART_ID)->dr, NULL, 0,
                        false);
}

void uart_send_bytes(uint8_t *buffer, size_t len) {
  task = xTaskGetCurrentTaskHandle();
  dma_channel_set_read_addr(dma_tx_chan, buffer, false);
  dma_channel_set_transfer_count(dma_tx_chan, len, true);
  int res = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
  return;
}
