#include "uart_task.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/dreq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/uart.h>
#include <stdint.h>
// Packet Structure
#define BUFFER_SIZE 8

// UART Definitions
#define UART_ID uart1
#define BAUD_RATE 3000000
#define TX_PIN 4
#define RX_PIN 5
typedef enum {
  HEADER1,
  HEADER2,
  PAYLOAD,
} state_t;

volatile static uint8_t packets[BUFFER_SIZE][PACKET_SIZE];
volatile static uint32_t head;
volatile static uint32_t tail;
volatile static int dma_rx_chan;
static state_t state;
static TaskHandle_t xTaskHandle = NULL;

// DMA Interrupt Handler
void on_uart_rx_dma() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (dma_channel_get_irq0_status(dma_rx_chan)) {
    dma_channel_acknowledge_irq0(dma_rx_chan);
    switch (state) {
    case HEADER1:
      if (packets[tail][0] == 0xA5) {
        state = HEADER2;
      }
      // Rewrite the address and restart as DMA automatically increments the
      // write address
      dma_channel_set_write_addr(dma_rx_chan, packets[tail], true);
      break;
    case HEADER2:
      if (packets[tail][0] == 0x55) {
        state = PAYLOAD;
        dma_channel_set_write_addr(dma_rx_chan, packets[tail], false);
        dma_channel_set_trans_count(dma_rx_chan, PACKET_SIZE, true);
      } else if (packets[tail][0] == 0xA5) {
        state = HEADER2;
        dma_channel_set_write_addr(dma_rx_chan, packets[tail], true);
      } else {
        state = HEADER1;
        dma_channel_set_write_addr(dma_rx_chan, packets[tail], true);
      }
      break;
    case PAYLOAD:
      // TODO: Add check to prevent the DMA from starting up a new request when
      // full. free_packet should set it up instead
      state = HEADER1;
      tail = (tail + 1) % BUFFER_SIZE;
      dma_channel_set_write_addr(dma_rx_chan, packets[tail], false);
      dma_channel_set_trans_count(dma_rx_chan, 1, true);

      if (xTaskHandle != NULL) {
        vTaskNotifyGiveFromISR(xTaskHandle, &xHigherPriorityTaskWoken);
      }
      break;
    default:
      state = HEADER1;
      break;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void start_uart_task(TaskHandle_t task) {
  // Initialize global state
  head = 0;
  tail = 0;
  state = HEADER1;
  xTaskHandle = task;

  // Initialize UART
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(RX_PIN, GPIO_FUNC_UART);

  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
  uart_set_fifo_enabled(UART_ID, true);

  // Enable DMA
  uint dreq_uart_rx = (UART_ID == uart0) ? DREQ_UART0_RX : DREQ_UART1_RX;
  dma_rx_chan = dma_claim_unused_channel(true);
  dma_channel_config_t c = dma_channel_get_default_config(dma_rx_chan);

  // Only increment on writes as we're writing to RAM
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);

  // Enable DMA interrupts
  channel_config_set_dreq(&c, dreq_uart_rx);
  irq_set_exclusive_handler(DMA_IRQ_0, on_uart_rx_dma);
  irq_set_enabled(DMA_IRQ_0, true);
  dma_channel_set_irq0_enabled(dma_rx_chan, true);

  // Start the DMA task
  dma_channel_configure(dma_rx_chan, &c, packets[0], &uart_get_hw(UART_ID)->dr,
                        1, true);
}

uint8_t *await_packet() {
  // Every proper packet will send a notifcation which acts like a semaphore
  // so we can just take a notifcation to determine if there's currently
  // a valid packet or we can just await for a packet
  ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
  return (uint8_t *)packets[head];
}

void free_packet() { head = (head + 1) % BUFFER_SIZE; }
