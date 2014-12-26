#ifndef _EASYINPUT_H
#define _EASYINPUT_H
#ifdef __cplusplus
extern "C" {
#endif

struct input_event;

/* Pass a string to the device name, or if you pass NULL,
 * it will default to "/dev/input/event1/"
 *
 * Will return 0 on success and 1 on failure
 */
int ei_setup(const char* device_name);

/* Pass in a pointer to a `struct input_event`. If an
 * event is found, the input_event will be filled with the
 * located event.
 *
 * It will return 1 when an event is found and 0 when none found
 * Example:
 *
 * struct input_event ev;
 * while (get_key_event(&ev)) {
 *     // ... use ev to do something
 * }
 */
int ei_get_key_event(struct input_event *ev);

#ifdef __cplusplus
}
#endif
#endif
