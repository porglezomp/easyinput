#include "easyinput.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static const char *dev = "/dev/input/event1";
static ssize_t n;
static int fd;
static fd_set writefds;
static struct timeval timeout;

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

        // We found a keypress!
        return 1;
    }
    // There are no keypresses ready to be read
    return 0;
}

