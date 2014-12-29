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

/* Will poll all available characters from the input
 * Use this if you only care about what keys are currently pressed
 */
void ei_poll_all();

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

/* Pass in a keycode from <linux/input.h> and return whether
 * or not it is currently pressed.
 *
 * It will return 1 if the key is pressed, and 0 if it's not
 */
int ei_key_down(int key);

/* Pass in a keycode from <linux/input.h> and return whether
 * or not it has been pressed (received a keydown event)
 * during this frame. (The time between `ei_frame_start` calls)
 *
 * It will return 1 if the key has been pressed, 0 otherwise
 */
int ei_frame_keypress(int key);

/* Will reset the state of all the keys to not pressed.
 * Useful for when you lose focus and are worries you missed the
 * key release event.
 */
void ei_reset_keys();

/* Will reset a single key to a not pressed state
 */
void ei_reset_key(int key);

/* Starts a "frame," of input, for use with `ei_frame_keypress`.
 * Internally, it resets the buffer of keydown events.
 */
void ei_frame_start();

#ifdef __cplusplus
}
#endif
#endif
