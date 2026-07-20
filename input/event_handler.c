#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

void start_listening(int device) {
  char path[64];
  snprintf(path, sizeof(path), "/dev/input/event%d", device);

  int fd;
  struct input_event ev;

  int repeatedFailure = 0;

  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open input device");
    return;
  }

  while (1) {
    if (read(fd, &ev, sizeof(struct input_event)) !=
        sizeof(struct input_event)) {

      perror("Failed to read input event");
      repeatedFailure++;
      if (repeatedFailure > 10) {
        break;
      }
      continue;
    }

    repeatedFailure = 0;
    printf("Event: time %ld.%06ld, type %d, code %d, value %d\n",
           ev.time.tv_sec, ev.time.tv_usec, ev.type, ev.code, ev.value);
  }

  close(fd);
}

void handler(struct input_event ev) {}
