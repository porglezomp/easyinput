#include "easyinput.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

static const char *dev = "/dev/input/event1";
static ssize_t n;
static int fd;
static fd_set writefds;
static struct timeval timeout;

#define NUM_KEYS 256
static int pressed_keys[NUM_KEYS];
static int keydown_keys[NUM_KEYS];

int ei_setup(const char* device_name) {
    // Use the default name
    if (device_name == NULL) device_name = dev;
    fd = open(device_name, O_RDONLY);
    if (fd == -1) return 1;
    return 0;
}

int ei_get_key_event(struct input_event *ev) {
    // select is allowed to modify the timeout,
    // so reset it to 10 microseconds
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    int result = select(fd+1, &writefds, NULL, NULL, &timeout);
    if (result == -1) {
        // Error!
        return 0;
    } else if (result) {
        n = read(fd, ev, sizeof *ev);

        // If it's got a wrong size of some other reason, give up
        if (n == (ssize_t)-1) return 0;
        // If the event struct is the wrong size, give up
        if (n != sizeof *ev) return 0;
        // Set the key as pressed or unpressed
        if (ev->type == EV_KEY && ev->value == 0 || ev->value == 1) {
            int key = ev->code;
            if (key >= 0 && key < NUM_KEYS) {
                pressed_keys[key] = ev->value;
                // Only ever promote from 0 to 1, never demote
                keydown_keys[key] |= ev->value;
            }
        }
        // We found a keypress!
        return 1;
    }
    // There are no keypresses ready to be read
    return 0;
}

void ei_poll_all() {
    struct input_event ev;
    // ei_get_key_event() will update the key pressed status,
    // so we run it until there are no events to get keypress state
    while (ei_get_key_event(&ev)) { }
}

int ei_key_down(int key) {
    if (key < 0 || key >= NUM_KEYS) return 0;
    return pressed_keys[key];
}

int ei_frame_keypress(int key) {
    if (key < 0 || key >= NUM_KEYS) return 0;
    return keydown_keys[key];
}

void ei_reset_keys() {
    memset(pressed_keys, 0, NUM_KEYS * sizeof(int));
}

void ei_reset_key(int key) {
    if (key >= 0 && key < NUM_KEYS) pressed_keys[key] = 0;
}

void ei_frame_start() {
    memset(keydown_keys, 0, NUM_KEYS * sizeof(int));
}

