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
  std::unordered_map<int, std::shared_ptr<PollDevice>> map_;
  int epoll_init();

public:
  EpollInstance() : epoll_fd_(epoll_init()), map_() {};
  // Adds a device to the epoll instance and starts polling
  // for reads. Use this function if your device needs to be deallocated
  // and want epoll to manage that deallocation
  void AddDevice(std::unique_ptr<PollDevice> dev);

  // Adds a device to the epoll instance and starts polling
  // for reads. Use this function if you need to manually
  // manage the lifetime of your device
  void AddDeviceUnmanaged(PollDevice *dev);

  // Enables polling for the write end of the fd as well
  void SetWrite(int fd, bool enable, PollDevice *instance);

  // Removes the device from the epoll instance. If the
  // device was added through AddDevice(), the device
  // will be deallocated as well
  void RemoveDevice(int fd);

  // General run loop that polls all epoll instances and
  // runs their on_poll() functions when polled
  void Run();
};

class EvdevPollDevice : public PollDevice {
private:
  int fd_;
  HidStructs::HidReport &rep_;
  WriteInterface &writer_;
  std::unique_ptr<HidStructs::EvdevHandler> handler_;
  libevdev *dev_;
  bool mark_delete_;

public:
  EvdevPollDevice(int fd, libevdev *dev, HidStructs::HidReport &rep,
                  std::unique_ptr<HidStructs::EvdevHandler> handler,
                  WriteInterface &writer)
      : fd_(fd), rep_(rep), dev_(dev), writer_(writer), mark_delete_(false),
        handler_(std::move(handler)) {}
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
