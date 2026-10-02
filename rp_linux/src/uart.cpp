#include "uart.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

Uart::Uart(const std::filesystem::path dev) {
  fd_ = open(dev.c_str(), O_RDWR | O_NOCTTY);
  if (fd_ <= -1) {
    throw std::runtime_error("Error opening serial port " + dev.string());
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));

  if (tcgetattr(fd_, &tty) != 0) {
    close(fd_);
    throw std::runtime_error("Unable to get attr for " + std::to_string(fd_));
  }

  cfsetspeed(&tty, B3000000);
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ECHONL;
  tty.c_lflag &= ~ISIG;

  tty.c_oflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 10;
  tcflush(fd_, TCIFLUSH);
  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    close(fd_);
    throw std::runtime_error("Unable to set attr for " + std::to_string(fd_));
  }
}

void Uart::write_bytes(const uint8_t *bytes, const uint8_t len) {
  int bytes_written = 0;
  if (bytes == nullptr || len <= 0) {
    return;
  }

  uint8_t header[] = {0xA5, 0x55, 0};
  header[2] = len;
  while (bytes_written < sizeof(header)) {
    int res =
        write(fd_, header + bytes_written, sizeof(header) - bytes_written);
    if (res < 0) {
      return;
    }
    bytes_written += res;
  }
  bytes_written = 0;

  while (bytes_written < len) {
    int res = write(fd_, bytes + bytes_written, len - bytes_written);
    if (res < 0) {
      return;
    }
    bytes_written += res;
  }
}

Uart::~Uart() { close(fd_); }
