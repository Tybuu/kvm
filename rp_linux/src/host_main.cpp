#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libudev.h>
#include <sys/epoll.h>
#include <unistd.h>

typedef enum { MONITOR, EVDEV } event_type_t;

typedef struct {
  event_type_t type;
  int fd;
  libevdev *dev;
} event_handler_t;

int main() {
  int fd;
  std::cout << "Hello there!\n";
  udev *udev = udev_new();
  if (udev == nullptr) {
    std::cerr << "udev unable to initialize\n";
    return 1;
  }
  udev_enumerate *enumerate = udev_enumerate_new(udev);
  if (enumerate == nullptr) {
    std::cerr << "Unable to initalize enumeration\n";
    return 1;
  }
  udev_enumerate_add_match_subsystem(enumerate, "input");
  int res = udev_enumerate_scan_devices(enumerate);
  if (res < 0) {
    std::cerr << "Error scanning devices";
    udev_enumerate_unref(enumerate);
    return 1;
  }

  struct udev_list_entry *entry = udev_enumerate_get_list_entry(enumerate);
  int epoll_fd = epoll_create1(0);

  struct epoll_event ev, events[20];
  while (entry != nullptr) {
    const char *name = udev_list_entry_get_name(entry);
    if (name != nullptr) {
      udev_device *dev = udev_device_new_from_syspath(udev, name);
      const char *model = udev_device_get_property_value(dev, "ID_MODEL");
      const char *devnode = udev_device_get_devnode(dev);
      if (devnode) {
        int fd = open(devnode, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
          std::cerr << "Unable to open " << devnode << std::endl;
        } else {
          libevdev *evdev = nullptr;
          if (libevdev_new_from_fd(fd, &evdev) < 0) {
            std::cerr << "Unable to initialize evdev for device " << devnode
                      << std::endl;
          } else {
            int is_key = libevdev_has_event_code(evdev, EV_KEY, KEY_A);
            int is_mouse = libevdev_has_event_code(evdev, EV_REL, REL_X);
            if (is_key) {
              std::cout << "Opened Keyboard device: "
                        << libevdev_get_name(evdev) << std::endl;
              event_handler_t *keyboard = new event_handler_t;
              keyboard->dev = evdev;
              keyboard->fd = fd;
              keyboard->type = EVDEV;

              ev.events = EPOLLIN | EPOLLRDHUP;
              ev.data.ptr = (void *)keyboard;
              epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
            } else if (is_mouse) {
              std::cout << "Opened Mouse device: " << libevdev_get_name(evdev)
                        << std::endl;
              event_handler_t *mouse = new event_handler_t;
              mouse->dev = evdev;
              mouse->fd = fd;
              mouse->type = EVDEV;

              ev.events = EPOLLIN | EPOLLRDHUP;
              ev.data.ptr = (void *)mouse;
              epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);

            } else {
              std::cout << "Opened Unspecified device: "
                        << libevdev_get_name(evdev) << std::endl;
            }
          }
        }
      }
      udev_device_unref(dev);
    }
    entry = udev_list_entry_get_next(entry);
  }
  udev_enumerate_unref(enumerate);

  struct udev_monitor *monitor = udev_monitor_new_from_netlink(udev, "udev");
  if (!monitor) {
    std::cout << "Unable to create monitor!" << std::endl;
    udev_unref(udev);
    return 1;
  }

  int monitor_fd = udev_monitor_get_fd(monitor);

  int flags = fcntl(monitor_fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  event_handler_t monitor_event;
  monitor_event.dev = nullptr;
  monitor_event.type = MONITOR;
  monitor_event.fd = monitor_fd;

  ev.data.ptr = &monitor_event;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, monitor_fd, &ev);
  while (true) {
    int nfds = epoll_wait(epoll_fd, events, 20, -1);

    for (int n = 0; n < nfds; ++n) {
      input_event ev;
      event_handler_t *dev = (event_handler_t *)events[n].data.ptr;
      if (dev->type == MONITOR) {
        struct udev_device *dev = udev_monitor_receive_device(monitor);
        if (dev) {
          const char *action = udev_device_get_action(dev);
          const char *syspath = udev_device_get_syspath(dev);
          const char *subsystem = udev_device_get_subsystem(dev);
          const char *devnode = udev_device_get_devnode(dev);
          {
            if (action &&
                (strcmp(action, "add") == 0 || strcmp(action, "remove") == 0)) {
              printf("[%s] Action\n", action);
              printf("  Subsystem: %s\n", subsystem ? subsystem : "N/A");
              printf("  Node Path: %s\n", devnode ? devnode : "N/A");
              printf("  Sys Path : %s\n\n", syspath);
            }

            udev_device_unref(dev);
          }
        }
      } else if (dev->type == EVDEV) {
        int res = libevdev_next_event(dev->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (res < 0) {
          continue;
        }
        std::cout << "Device: " << libevdev_get_name(dev->dev)
                  << " | Type: " << libevdev_event_type_get_name(ev.type)
                  << " | Code: "
                  << libevdev_event_code_get_name(ev.type, ev.code)
                  << " | Value: " << ev.value << std::endl;
      }
    }
  }
  return 0;
}
