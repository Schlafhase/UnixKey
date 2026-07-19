#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define PROJECT_NAME "UnixKey"

// event4 is the keyboard on my pc

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <event-number>\n", argv[0]);
    return 1;
  }

  char path[64];
  snprintf(path, sizeof(path), "/dev/input/event%s", argv[1]);
  printf("path: %s", path);

  int fd;
  struct input_event ev;

  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open input device");
    return 1;
  }

  while (1) {
    if (read(fd, &ev, sizeof(struct input_event)) !=
        sizeof(struct input_event)) {

      perror("Failed to read input event");
      break;
    }

    printf("Event: time %ld.%06ld, type %d, code %d, value %d\n",
           ev.time.tv_sec, ev.time.tv_usec, ev.type, ev.code, ev.value);
  }

  close(fd);
  return 0;
}
