#pragma once

#include "poll.hpp"
#include <cstdint>
#include <filesystem>
const size_t BUFFER_SIZE = 1024;
class Uart : public Poll::WriteInterface {
private:
  int fd_;

  void flush();

public:
  Uart(const Uart &) = delete;

  Uart &operator=(const Uart &) = delete;

  Uart(const std::filesystem::path dev);

  ~Uart();

  void WriteBytes(const std::uint8_t *bytes, const uint8_t len) override;

  // TODO: Implement read packet
  // void read_packet(uint8_t *bytes, const size_t len);
};
