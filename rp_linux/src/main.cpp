#include "poll.hpp"
#include "uart.hpp"
#include <cstring>
#include <fcntl.h>
#include <libudev.h>
#include <termios.h>
#include <unistd.h>

int main() {
  HidStructs::HidReport rep;
  Poll::EpollInstance epoll = Poll::EpollInstance();
  Uart uart = Uart("/dev/ttyAMA0");
  // Poll::CoutWrite uart = Poll::CoutWrite();
  auto uvdev = std::make_unique<Poll::UdevPollDevice>(epoll, rep, uart);
  epoll.AddDevice(std::move(uvdev));

  epoll.Run();

  return 0;
}
