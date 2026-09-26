#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <libevdev/libevdev.h>
#include <unistd.h>

int main() {
  int fd;
  for (int i = 0; i < 32; i++) {
    std::string path = "/dev/input/event" + std::to_string(i);

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);

    if (fd < 0) {
      continue;
    }
    struct libevdev *dev = NULL;
    if (libevdev_new_from_fd(fd, &dev) < 0) {
      close(fd);
      continue;
    }
    std::cout << "Found Device: " << path << " -> " << libevdev_get_name(dev)
              << "\n";
    libevdev_free(dev);
    close(fd);
  }

  return 0;
}
