#ifndef UART_TASK_H
#define UART_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
typedef struct {
  uint16_t len;
  uint8_t *packet;
} uart_packet_t;
// Start the UART peripheral and its DMA task. The UART peripheral will keep
// receiving in the background and the packets can be processed with
// await_packet(). To free the packet, call free_packet()
void start_uart_task(TaskHandle_t handle);

// Blocks the current task until a valid packet is available. start_uart_task()
// must be called before calling this function
uart_packet_t await_packet();

// Frees the packet from await_packet. Must be called after await_packet returns
void free_packet(uart_packet_t *packet);
#endif // UART_TASK_H
