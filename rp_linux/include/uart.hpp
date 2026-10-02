#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
class Uart {
private:
  int fd_ = -1;

public:
  Uart(const Uart &) = delete;

  Uart &operator=(const Uart &) = delete;

  Uart(const std::filesystem::path dev);

  ~Uart();

  void write_bytes(const uint8_t *bytes, const uint8_t len);

  // TODO: Implement read packet
  // void read_packet(uint8_t *bytes, const size_t len);
};
