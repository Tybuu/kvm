#include "uart.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libudev.h>
#include <ratio>
#include <termios.h>
#include <thread>
#include <unistd.h>

int main() {
  std::cout << "Hello there!\n";

  // TODO: Use libudev to enumerate through all possible devices
  Uart uart = Uart("/dev/ttyAMA0");
  while (true) {
    uint8_t bytes[] = {5, 4, 9, 2, 3, 8};
    uart.write_bytes(bytes, sizeof(bytes));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Wrote bytes!\n";
  }
  return 0;
}
