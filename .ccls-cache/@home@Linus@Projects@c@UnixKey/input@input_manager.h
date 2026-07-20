#ifndef INPUT_MANAGER_H_INCLUDED
#define INPUT_MANAGER_H_INCLUDED

#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX_LISTENERS 255

typedef void key_event_listener_fn(struct input_event);

typedef struct {
  int fd;
  key_event_listener_fn *listeners[MAX_LISTENERS];
  size_t listener_count;
} evdev_manager_t;

// Basically constructor
int evdev_manager_init(evdev_manager_t *em, int device_number);

// Add an event listener to the input manager
int evdev_manager_add_listener(evdev_manager_t *em, key_event_listener_fn fn,
                               void *user_data);

// Calls the event listerners if the input device has new events
void evdev_manager_update(evdev_manager_t *em);

void evdev_manager_close(evdev_manager_t *em);

#endif
