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
// Packet Structure
#define BUFFER_SIZE_LOG2 13
#define BUFFER_SIZE (1 << BUFFER_SIZE_LOG2)

#define HEADER_SIZE 3

// UART Definitions
#define UART_ID uart1
#define BAUD_RATE 3000000
#define TX_PIN 4
#define RX_PIN 5

#define NUM_PACKETS 32
typedef enum {
  GET_PACKET,
  HEADER1,
  HEADER2,
  LEN,
  PAYLOAD,
} state_t;

volatile static uint8_t packets[BUFFER_SIZE]
    __attribute__((aligned(BUFFER_SIZE)));
volatile static uint32_t head;
volatile static uint32_t tail;
volatile static int dma_rx_chan;

static QueueHandle_t xFreePacketQueue;
static uart_packet_t uart_packets[NUM_PACKETS];
static state_t state;
static uart_packet_t *cur_packet;
static uint16_t bytes_read;

void on_uart_rx_dma() {
  if (dma_channel_get_irq0_status(dma_rx_chan)) {
    dma_channel_acknowledge_irq0(dma_rx_chan);
    dma_channel_set_transfer_count(dma_rx_chan, 0xFFFFFFFF, true);
  }
}

void start_uart_task() {
  // Initialize global state
  head = 0;
  tail = 0;
  state = GET_PACKET;
  cur_packet = NULL;

  xFreePacketQueue = xQueueCreate(NUM_PACKETS, sizeof(uart_packet_t *));
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

  channel_config_set_ring(&c, true, BUFFER_SIZE_LOG2);

  // Enable DMA interrupts
  channel_config_set_dreq(&c, dreq_uart_rx);
  irq_set_exclusive_handler(DMA_IRQ_0, on_uart_rx_dma);
  irq_set_enabled(DMA_IRQ_0, true);
  dma_channel_set_irq0_enabled(dma_rx_chan, true);

  for (int i = 0; i < NUM_PACKETS; i++) {
    uart_packet_t *packet = &uart_packets[i];
    xQueueSend(xFreePacketQueue, &packet, 0);
  }

  // Start the DMA task
  dma_channel_configure(dma_rx_chan, &c, &packets[tail],
                        &uart_get_hw(UART_ID)->dr, 0xFFFFFFFF, true);
}

static void get_bytes_blocking(uint8_t *buffer, size_t len) {
  for (int i = 0; i < len; i++) {

    uint32_t current_transfers = dma_hw->ch[dma_rx_chan].transfer_count;
    tail = (0xFFFFFFFF - current_transfers) % BUFFER_SIZE;

    while (head == tail) {
      current_transfers = dma_hw->ch[dma_rx_chan].transfer_count;
      tail = (0xFFFFFFFF - current_transfers) % BUFFER_SIZE;
    }
    buffer[i] = packets[head];
    head = (head + 1) % BUFFER_SIZE;
  }
}

static int get_bytes(uint8_t *buffer, size_t len) {
  for (int i = 0; i < len; i++) {
    uint32_t current_transfers = dma_hw->ch[dma_rx_chan].transfer_count;
    tail = (0xFFFFFFFF - current_transfers) % BUFFER_SIZE;

    if (head == tail) {
      return i;
    }
    buffer[i] = packets[head];
    head = (head + 1) % BUFFER_SIZE;
  }
  return len;
}

uart_packet_t *get_packet() {
  while (true) {
    switch (state) {
    case GET_PACKET: {
      if (xQueueReceive(xFreePacketQueue, &cur_packet, 0) == pdTRUE) {
        state = HEADER1;
      } else {
        return NULL;
      }
      break;
    }
    case HEADER1: {
      uint8_t cur_byte = 0;
      int res = get_bytes(&cur_byte, 1);
      if (res != 1) {
        return NULL;
      }
      if (cur_byte == 0xA5) {
        state = HEADER2;
      }
      break;
    }
    case HEADER2: {
      uint8_t cur_byte = 0;
      int res = get_bytes(&cur_byte, 1);
      if (res != 1) {
        return NULL;
      }
      if (cur_byte == 0x55) {
        state = LEN;
      } else if (cur_byte == 0xA5) {
        state = HEADER2;
      } else {
        state = HEADER1;
      }
      break;
    }
    case LEN: {
      uint8_t cur_byte = 0;
      int res = get_bytes(&cur_byte, 1);
      if (res != 1) {
        return NULL;
      }
      if (cur_byte == 0) {
        state = HEADER1;
      } else {
        cur_packet->len = cur_byte;
        bytes_read = 0;
        state = PAYLOAD;
      }
      break;
    }
    case PAYLOAD: {
      int res = get_bytes(&cur_packet->data[bytes_read],
                          cur_packet->len - bytes_read);
      bytes_read += res;
      if (bytes_read == cur_packet->len) {
        state = GET_PACKET;
        uart_packet_t *res = cur_packet;
        cur_packet = NULL;
        return res;
      } else {
        return NULL;
      }
      break;
    }
    default:
      return NULL;
    }
  }
}

uart_packet_t *get_packet_block() {
  uart_packet_t *packet = NULL;
  while (true) {
    switch (state) {
    case GET_PACKET: {
      xQueueReceive(xFreePacketQueue, &packet, portMAX_DELAY);
      state = HEADER1;
      break;
    }
    case HEADER1: {
      uint8_t byte = 0;
      get_bytes_blocking(&byte, 1);
      if (byte == 0xA5) {
        state = HEADER2;
      }
      break;
    }
    case HEADER2: {
      uint8_t byte = 0;
      get_bytes_blocking(&byte, 1);
      if (byte == 0x55) {
        state = LEN;
      } else if (byte == 0xA5) {
        state = HEADER2;
      } else {
        state = HEADER1;
      }
      break;
    }
    case LEN: {
      uint8_t byte = 0;
      get_bytes_blocking(&byte, 1);
      if (byte == 0) {
        state = HEADER1;
      } else {
        packet->len = byte;
        state = PAYLOAD;
      }
      break;
    }
    case PAYLOAD: {
      get_bytes_blocking(packet->data, packet->len);
      state = GET_PACKET;
      return packet;
      break;
    }
    }
  }
}

void print_backlog() {
  uint32_t wr =
      ((uintptr_t)dma_hw->ch[dma_rx_chan].write_addr - (uintptr_t)packets) %
      BUFFER_SIZE;
  uint32_t backlog = (wr - head) % BUFFER_SIZE;
  printf("Packet Backlog: %d\n", backlog);
}

void free_packet(uart_packet_t *packet) {
  xQueueSend(xFreePacketQueue, &packet, 0);
}
