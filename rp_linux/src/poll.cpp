#include "hid_commands.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libevdev/libevdev.h>
#include <libudev.h>
#include <memory>
#include <poll.hpp>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

const size_t NUM_EVENTS = 20;
namespace Poll {

void CoutWrite::WriteBytes(const std::uint8_t *bytes, const uint8_t len) {
  std::cout << "Len: " << static_cast<int>(len) << " | Bytes: [";
  for (int i = 0; i < len; i++) {
    if (i < len - 1) {
      std::cout << static_cast<int>(bytes[i]) << ", ";
    } else {
      std::cout << static_cast<int>(bytes[i]) << "]\n";
    }
  }
}

int EpollInstance::epoll_init() {
  int fd = epoll_create1(0);
  if (fd < 0) {
    throw std::runtime_error("epoll creation failed");
  }
  return fd;
}

void EpollInstance::AddReadDevice(std::unique_ptr<Poll::PollDevice> dev) {
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLRDHUP;
  ev.data.ptr = (void *)dev.get();
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, dev->fd(), &ev);
  map_[dev->fd()] = std::move(dev);
}

void EpollInstance::RemoveDevice(int fd) {
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
  std::cout << "Removing fd " << fd << std::endl;
  map_.erase(fd);
}

void EpollInstance::Run() {
  // TODO: Add Remove Queue
  epoll_event events[NUM_EVENTS];
  std::vector<int> delete_queue;
  delete_queue.reserve(NUM_EVENTS);
  while (true) {
    int nfds = epoll_wait(epoll_fd_, events, NUM_EVENTS, -1);
    for (int n = 0; n < nfds; ++n) {
      PollDevice *dev = static_cast<PollDevice *>(events[n].data.ptr);
      dev->OnPoll();
      if (dev->ShouldDelete()) {
        delete_queue.push_back(dev->fd());
      }
    }
    for (auto &fd : delete_queue) {
      RemoveDevice(fd);
    }
    delete_queue.clear();
  }
}

void EvdevPollDevice::OnPoll() {
  int res;
  input_event ev;
  std::uint8_t buffer[512];
  do {
    res = libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (res == LIBEVDEV_READ_STATUS_SUCCESS) {
      uint8_t res = rep_.GenerateReport(ev, buffer);
      if (res != 0) {
        writer_.WriteBytes(buffer, res);
      }
    } else if (res == LIBEVDEV_READ_STATUS_SYNC) {
      std::cout << "Device" << libevdev_get_name(dev_) << "unsynced\n";
    } else if (res == -ENODEV) {
      std::cout << "Device " << libevdev_get_name(dev_) << " Deleted\n";
      mark_delete_ = true;
    }
  } while (res == LIBEVDEV_READ_STATUS_SUCCESS ||
           res == LIBEVDEV_READ_STATUS_SYNC);
}

UdevPollDevice::UdevPollDevice(EpollInstance &poll, HidStructs::HidReport &rep,
                               WriteInterface &writer)
    : poll_(poll), rep_(rep), writer_(writer) {
  udev_ = udev_new();
  if (udev_ == nullptr) {
    throw std::runtime_error("Unable to initialize udev");
  }
  udev_enumerate *enumerate = udev_enumerate_new(udev_);
  if (enumerate == nullptr) {
    throw std::runtime_error("Unable to initialize udev enumeratation");
  }
  udev_enumerate_add_match_subsystem(enumerate, "input");
  int res = udev_enumerate_scan_devices(enumerate);
  if (res < 0) {
    udev_enumerate_unref(enumerate);
    throw std::runtime_error("Unable to enumerate");
  }

  struct udev_list_entry *entry = udev_enumerate_get_list_entry(enumerate);
  while (entry != nullptr) {
    const char *name = udev_list_entry_get_name(entry);
    if (name != nullptr) {
      udev_device *dev = udev_device_new_from_syspath(udev_, name);
      const char *model = udev_device_get_property_value(dev, "ID_MODEL");
      const char *devnode = udev_device_get_devnode(dev);
      if (devnode) {
        int fd = open(devnode, O_RDONLY | O_NONBLOCK);
        int rep[2] = {0, 0};
        ioctl(fd, EVIOCSREP, rep);
        if (fd < 0) {
          std::cerr << "Unable to open " << devnode << std::endl;
        } else {
          libevdev *evdev = nullptr;
          if (libevdev_new_from_fd(fd, &evdev) >= 0) {
            int is_key = libevdev_has_event_code(evdev, EV_KEY, KEY_A);
            int is_mouse = libevdev_has_event_code(evdev, EV_REL, REL_X);
            if (is_key || is_mouse) {
              std::cout << "Opened Keyboard device: "
                        << libevdev_get_name(evdev) << std::endl;
              std::unique_ptr<EvdevPollDevice> ptr =
                  std::make_unique<EvdevPollDevice>(fd, evdev, rep_, writer_);
              poll_.AddReadDevice(std::move(ptr));
            } else if (is_mouse) {
              std::cout << "Opened Mouse device: " << libevdev_get_name(evdev)
                        << std::endl;
              std::unique_ptr<EvdevPollDevice> ptr =
                  std::make_unique<EvdevPollDevice>(fd, evdev, rep_, writer_);
              poll_.AddReadDevice(std::move(ptr));
            }
          }
        }
      }
      udev_device_unref(dev);
    }
    entry = udev_list_entry_get_next(entry);
  }
  udev_enumerate_unref(enumerate);
  mon_ = udev_monitor_new_from_netlink(udev_, "udev");
  if (mon_ == nullptr) {
    udev_unref(udev_);
    throw std::runtime_error("Unable to create monitor!");
  }

  fd_ = udev_monitor_get_fd(mon_);
  int flags = fcntl(fd_, F_GETFL, 0);
  fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

void UdevPollDevice::OnPoll() {
  udev_device *dev = udev_monitor_receive_device(mon_);
  while (dev != nullptr) {
    const char *action = udev_device_get_action(dev);
    const char *syspath = udev_device_get_syspath(dev);
    const char *subsystem = udev_device_get_subsystem(dev);
    const char *devnode = udev_device_get_devnode(dev);
    {
      if (action && devnode && (strcmp(action, "add") == 0) &&
          (strncmp(devnode, "/dev/input", 10) == 0)) {
        int fd = open(devnode, O_RDONLY | O_NONBLOCK);
        int rep[2] = {0, 0};
        ioctl(fd, EVIOCSREP, rep);
        if (fd >= 0) {
          libevdev *evdev = nullptr;
          if (libevdev_new_from_fd(fd, &evdev) < 0) {
            std::cerr << "Unable to initialize evdev for device " << devnode
                      << std::endl;
          } else {
            int is_key = libevdev_has_event_code(evdev, EV_KEY, KEY_A);
            int is_mouse = libevdev_has_event_code(evdev, EV_REL, REL_X);
            if (is_key || is_mouse) {
              std::cout << "Opened Keyboard device: "
                        << libevdev_get_name(evdev) << std::endl;
              std::unique_ptr<EvdevPollDevice> ptr =
                  std::make_unique<EvdevPollDevice>(fd, evdev, rep_, writer_);
              poll_.AddReadDevice(std::move(ptr));
            } else if (is_mouse) {
              std::cout << "Opened Mouse device: " << libevdev_get_name(evdev)
                        << std::endl;
              std::unique_ptr<EvdevPollDevice> ptr =
                  std::make_unique<EvdevPollDevice>(fd, evdev, rep_, writer_);
              poll_.AddReadDevice(std::move(ptr));
            }
          }
        }
      }

      udev_device_unref(dev);
    }
    dev = udev_monitor_receive_device(mon_);
  }
}
} // namespace Poll
