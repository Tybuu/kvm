#include "middle.hpp"
#include "poll.hpp"
#include <cerrno>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libudev.h>
#include <mutex>
#include <stdexcept>
#include <unistd.h>

namespace Middle {
Writer::~Writer() { close(fd_); }
Writer::Writer() : head_(0), tail_(0), buffer_{}, cv_{}, mutex_{} {
  udev *udev = udev_new();
  fd_ = -1;
  if (udev == nullptr) {
    throw std::runtime_error("Unable to initialize udev");
  }
  udev_enumerate *enumerate = udev_enumerate_new(udev);
  if (enumerate == nullptr) {
    throw std::runtime_error("Unable to initialize udev enumeratation");
  }
  udev_enumerate_add_match_subsystem(enumerate, "hidraw");
  int res = udev_enumerate_scan_devices(enumerate);
  if (res < 0) {
    udev_enumerate_unref(enumerate);
    throw std::runtime_error("Unable to enumerate");
  }

  struct udev_list_entry *entry = udev_enumerate_get_list_entry(enumerate);
  while (entry != nullptr) {
    const char *name = udev_list_entry_get_name(entry);
    if (name != nullptr) {
      udev_device *dev = udev_device_new_from_syspath(udev, name);
      const char *model = udev_device_get_property_value(dev, "ID_MODEL");
      const char *devnode = udev_device_get_devnode(dev);
      if (devnode) {
        const char *pid = udev_device_get_property_value(dev, "ID_MODEL_ID");
        const char *vid = udev_device_get_property_value(dev, "ID_VENDOR_ID");
        if (strncmp(pid, "0728", 4) == 0 && strncmp(vid, "0727", 4) == 0) {
          int fd = open(devnode, O_RDWR);
          if (fd < 0) {
            throw std::runtime_error("hidraw unable to open");
          }
          fd_ = fd;
          udev_device_unref(dev);
          break;
        }
      }
      udev_device_unref(dev);
    }
    entry = udev_list_entry_get_next(entry);
  }
  // TODO: Figure out a way to middlewriter to function without an actual
  // instance instead of crashing
  if (fd_ == -1) {
    throw std::runtime_error("No middlewriter available");
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
};

void Writer::WriteBytes(const std::uint8_t *bytes, const uint8_t len) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_[tail_][0] = 0;
    for (int i = 0; i < len; i++) {
      buffer_[tail_][i + 1] = bytes[i];
    }
    tail_ = (tail_ + 1) % NUM_PACKETS;
  }
  cv_.notify_one();
}

void Writer::Run() {
  std::uint8_t buffers[NUM_PACKETS][PACKET_SIZE];
  int count = 0;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return head_ != tail_; });
      count = 0;
      while (head_ != tail_) {
        memcpy(buffers[count], buffer_[head_], PACKET_SIZE);
        head_ = (head_ + 1) % NUM_PACKETS;
        count += 1;
      }
    }
    for (int i = 0; i < count; i++) {
      while (true) {
        int res = write(fd_, buffers[i], PACKET_SIZE);
        // TODO: Handle writer failures
        if (res < 0 && errno != EAGAIN) {
          throw std::runtime_error("HidWriter failed!");
        } else {
          break;
        }
      }
    }
  }
}
}; // namespace Middle
