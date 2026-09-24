#ifndef EMULATE_H
#define EMULATE_H

#include "report_types.h"
#include <stddef.h>
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

#define KEY_COMMAND_SIZE 3
typedef struct {
  uint8_t keycode;
  key_state_t state;
} key_command_t;

#define MOUSE_COMMAND_SIZE 3
typedef struct {
  int8_t x;
  int8_t y;
} mouse_command_t;

#define BUTTON_COMMAND_SIZE 3
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

inline hid_command_t keyboard_command(uint8_t keycode, key_state_t state) {
  return (hid_command_t){.type = KEYBOARD_COMMAND,
                         .key = {.keycode = keycode, .state = state}};
}

inline hid_command_t mouse_command(int8_t x, int8_t y) {
  return (hid_command_t){.type = MOUSE_COMMAND, .mouse = {.x = x, .y = y}};
}

inline hid_command_t button_command(uint8_t keycode, key_state_t state) {
  return (hid_command_t){.type = MOUSE_BUTTON_COMMAND,
                         .buttons = {.keycode = keycode, .state = state}};
}

// Attempts to generate a report with the passed in command. If changes are made
// to the state, the passed in report will have the changed state. The function
// returns true if there's a new report that should be sent
bool generate_report(hid_state_t *state, hid_generic_report_t *report,
                     hid_command_t command);

// Deserializes the byte stream into the passed in command. Returns true
// if properly deserialized
bool deserialize_command(uint8_t *bytes, uint16_t len, hid_command_t *command);

#endif // EMULATE_H
