#ifndef UART_TASK_H
#define UART_TASK_H

#include <stdint.h>
#define PACKET_MAX_SIZE 128
typedef struct {
  uint16_t len;
  uint8_t data[PACKET_MAX_SIZE];
} uart_packet_t;
// Start the UART peripheral and its DMA task. The UART peripheral will keep
// receiving in the background and the packets can be processed with
// await_packet(). To free the packet, call free_packet()
void start_uart_task();

// Blocks the current task until a valid packet is available. start_uart_task()
// must be called before calling this function
uart_packet_t *get_packet_block();

uart_packet_t *get_packet();
// Frees the packet from await_packet. Must be called after await_packet returns
void free_packet(uart_packet_t *packet);

void print_backlog();
#endif // UART_TASK_H
