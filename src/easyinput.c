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

int find_device();

int ei_setup(const char* device_name) {
  // Try to figure out the name if we recieved NULL
  if (device_name == NULL) {
    fd = find_device();
  } else {
    fd = open(device_name, O_RDONLY);
  }
  return (fd == -1);
}

int ei_teardown() {
  close(fd);
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

int find_device() {
  // We must examine the input devices
  int fd = open("/proc/bus/input/devices", O_RDONLY);
  int event_no = -1;

  enum {
    DEAD_LINE,
    READ_LINE,
    BEGIN_LINE
  };

  int i, count = 0, line_state = BEGIN_LINE;
  char fbuffer[100];
  char last_char = '0';
  // Process the file in chunks of 100 characters at a time
  while ((count = read(fd, fbuffer, 100)) > 0) {
    for (i = 0; i < count; i++) {
      char letter = fbuffer[i];
      // If we reach a newline, start processing the new line
      if (letter == '\n') {
	line_state = BEGIN_LINE;
	continue;
      } else if (line_state == DEAD_LINE) {
	// If the line has already been disqualified, skip characters
	continue;
      }

      // If we're just beginning a line
      if (line_state == BEGIN_LINE) {
	// Keep reading the line if the first character is 'N', otherwise skip
	if (letter == 'N') {
	  line_state = READ_LINE;
	  // We're searching in this line, if it matches event_no will produce
	  // the device name: event0, event1, etc.
	  event_no += 1;
	} else {
	  line_state = DEAD_LINE;
	}
	continue;
      }

      // If we've made it to here, we're processing a line
      // What follows is an ad-hoc state machine to match [Kk](eyboard|db)
      switch (last_char) {
      case '0':
	if (letter == 'k' || letter == 'K') last_char = 'k';
	else last_char = '0';
	break;
      case 'k':
	if (letter == 'e' || letter == 'd') last_char = letter;
	else last_char = '0';
	break;
      case 'd':
	// 1 signals that a match has been found (matches [Kk]db)
	if (letter == 'b') last_char = '1';
	else last_char = '0';
	break;
	// This macro handles checking for a transition from f -> t
#define STATE_FROM_TO(f, t) case f:\
	if (letter == t) last_char = t;\
	else last_char = '0';\
	break;
	
	STATE_FROM_TO('e', 'y');
	STATE_FROM_TO('y', 'b');
	STATE_FROM_TO('b', 'o');
	STATE_FROM_TO('o', 'a');
	STATE_FROM_TO('a', 'r');
	
      case 'r':
	// 1 signals that a match has been found (matches [Kk]eyboard)
	if (letter == 'd') last_char = '1';
	else last_char = '0';
	break;
      }

      // A match was found on this line, break and use event_no
      // to open the device file
      if (last_char == '1') {
	break;
      }
    }
  }

  close(fd);
  // If we found an input device that looks like a keyboard
  if (event_no != -1) {
    // Construct the name of the device...
    char fname[20];
    snprintf(fname, 20, "/dev/input/event%i", event_no);
    // ...and then return a file handle pointing to it
    return open(fname, O_RDONLY);
  }
  
  return -1;
}

