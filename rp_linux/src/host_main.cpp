#include "hid_commands.hpp"
#include "poll.hpp"
#include <cstring>
#include <fcntl.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libudev.h>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>

int main() {
  HidStructs::HidReport rep;
  Poll::EpollInstance epoll = Poll::EpollInstance();
  Poll::CoutWrite writer = Poll::CoutWrite();
  auto uvdev = std::make_unique<Poll::UdevPollDevice>(epoll, rep, writer);
  epoll.AddDevice(std::move(uvdev));

  epoll.Run();

  return 0;
}
