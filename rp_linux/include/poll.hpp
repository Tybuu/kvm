#pragma once

#include "hid_commands.hpp"
#include <libevdev/libevdev.h>
#include <libudev.h>
#include <memory>
#include <unistd.h>
#include <unordered_map>
namespace Poll {
class WriteInterface {
public:
  virtual void WriteBytes(const std::uint8_t *bytes, const uint8_t len) = 0;
};

class CoutWrite : public WriteInterface {
public:
  void WriteBytes(const std::uint8_t *bytes, const uint8_t len) override;
};
class PollDevice {
public:
  virtual int fd() = 0;
  virtual void OnPoll() = 0;
  virtual bool ShouldDelete() = 0;
  virtual ~PollDevice() = default;
};

class EpollInstance {
private:
  int epoll_fd_;
  std::unordered_map<int, std::unique_ptr<PollDevice>> map_;
  int epoll_init();

public:
  EpollInstance() : epoll_fd_(epoll_init()), map_() {};
  void AddReadDevice(std::unique_ptr<PollDevice> dev);
  void AddWriteDevice(std::unique_ptr<PollDevice> dev);
  void RemoveDevice(int fd);
  void Run();
};

class EvdevPollDevice : public PollDevice {
private:
  int fd_;
  HidStructs::HidReport &rep_;
  WriteInterface &writer_;
  libevdev *dev_;
  bool mark_delete_;

public:
  EvdevPollDevice(int fd, libevdev *dev, HidStructs::HidReport &rep,
                  WriteInterface &writer)
      : fd_(fd), rep_(rep), dev_(dev), writer_(writer), mark_delete_(false) {}
  int fd() override { return fd_; }
  void OnPoll() override;
  bool ShouldDelete() override { return mark_delete_; }
  ~EvdevPollDevice() override {
    libevdev_free(dev_);
    close(fd_);
  };
};
class UdevPollDevice : public PollDevice {
private:
  udev *udev_;
  WriteInterface &writer_;
  udev_monitor *mon_;
  HidStructs::HidReport &rep_;
  int fd_;
  EpollInstance &poll_;

public:
  UdevPollDevice(Poll::EpollInstance &poll, HidStructs::HidReport &rep,
                 WriteInterface &writer);
  int fd() override { return fd_; }
  void OnPoll() override;
  bool ShouldDelete() override { return false; }
  ~UdevPollDevice() override {
    udev_monitor_unref(mon_);
    udev_unref(udev_);
  }
};
} // namespace Poll
