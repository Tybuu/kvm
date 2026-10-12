#ifndef UART_TASK_H
#define UART_TASK_H

#include <stddef.h>
#include <stdint.h>
// Enables and initialize the UART peripheral
void uart_init_task();

void uart_send_bytes(uint8_t *buffer, size_t len);
#endif // UART_TASK_H
