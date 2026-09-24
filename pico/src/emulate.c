#include "emulate.h"
#include "report_types.h"
#include <stdint.h>
#include <string.h>
bool generate_report(hid_state_t *state, hid_generic_report_t *report,
                     hid_command_t command) {
  switch (command.type) {
  case KEYBOARD_COMMAND: {
    bool changed;
    if (command.key.state == PRESSED) {
      changed = nkro_press_key(&state->nkro, command.key.keycode);
    } else {
      changed = nkro_release_key(&state->nkro, command.key.keycode);
    }
    if (changed) {
      set_generic_nkro(report, &state->nkro);
    }
    return changed;
    break;
  }
  case MOUSE_COMMAND: {
    mouse_clear_delta(&state->mouse);
    mouse_move(&state->mouse, command.mouse.x, command.mouse.y,
               command.mouse.wheel);
    // We only want to send mouse reports if there's some delta change in the
    // command, would be a no-op otherwise
    if (command.mouse.x != 0 || command.mouse.y != 0) {
      set_generic_mouse(report, &state->mouse);
      return true;
    } else {
      return false;
    }
    break;
  }
  case MOUSE_BUTTON_COMMAND: {
    bool changed;
    mouse_clear_delta(&state->mouse);
    if (command.buttons.state == PRESSED) {
      changed = mouse_press_button(&state->mouse, command.key.keycode);
    } else {
      changed = mouse_release_button(&state->mouse, command.key.keycode);
    }
    if (changed) {
      set_generic_mouse(report, &state->mouse);
    }
    return changed;
    break;
  }
  }
}

bool deserialize_command(uint8_t *bytes, uint16_t len, hid_command_t *command) {
  if (len <= 1 || command == NULL) {
    return false;
  }
  switch (bytes[0]) {
  case KEYBOARD_COMMAND: {
    if (len != KEY_COMMAND_SIZE) {
      return false;
    }
    command->type = KEYBOARD_COMMAND;
    command->key.keycode = bytes[1];
    command->key.state = bytes[2];
    return true;
    break;
  }
  case MOUSE_COMMAND: {
    if (len != MOUSE_COMMAND_SIZE) {
      return false;
    }
    command->type = MOUSE_COMMAND;
    command->mouse.x = bytes[1];
    command->mouse.y = bytes[2];
    return true;
    break;
  }
  case MOUSE_BUTTON_COMMAND: {
    if (len != BUTTON_COMMAND_SIZE) {
      return false;
    }
    command->type = MOUSE_BUTTON_COMMAND;
    command->buttons.keycode = bytes[1];
    command->buttons.state = bytes[2];
    return true;
    break;
  }
  default:
    return false;
  }
}
