#ifndef EMULATE_H
#define EMULATE_H

#include "class/hid/hid.h"
#include "usb_descriptors.h"
#include <stdint.h>
typedef struct {
  hid_report_nkro_t nkro;
  hid_mouse_report_t mouse;
} hid_state_t;

typedef enum {
  KEYBOARD_COMMAND = 0,
  MOUSE_COMMAND = 1,
  MOUSE_BUTTON_COMMAND = 2,
} hid_command_type_t;

typedef enum { RELEASED = 0, PRESSED = 1 } key_state_t;

typedef struct {
  uint8_t keycode;
  key_state_t state;
} key_command_t;

typedef struct {
  uint8_t x;
  uint8_t y;
} mouse_command_t;

typedef struct {
  uint8_t keycode;
  key_state_t state;
} mouse_buttom_command_t;

typedef struct {
  hid_command_type_t type;
  union {
    key_command_t key;
    mouse_command_t mouse;
    mouse_buttom_command_t buttons;
  };
} hid_command_t;

// Attempts to generate a report with the passed in command. If changes are made
// to the state, the passed in report will have the changed state. The
// programmer will be notified of
bool generate_report(hid_state_t *state, hid_generic_report_t *report,
                     hid_command_t command);
#endif // EMULATE_H
