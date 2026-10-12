#pragma once

#include "poll.hpp"
#include <condition_variable>
#include <cstdint>

namespace Middle {
class Writer : public Poll::WriteInterface {
private:
  static const size_t PACKET_SIZE = 65;
  static const size_t NUM_PACKETS = 32;
  int fd_;
  std::uint8_t buffer_[NUM_PACKETS][PACKET_SIZE];
  std::size_t head_;
  std::size_t tail_;
  std::mutex mutex_;
  std::condition_variable cv_;

public:
  Writer();

  Writer(const Writer &) = delete;

  Writer &operator=(const Writer &) = delete;

  ~Writer();
  void WriteBytes(const std::uint8_t *bytes, const uint8_t len) override;

  void Run();
};

}; // namespace Middle
