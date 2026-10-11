#include <cstdint>
#include <cstring>
#include <hid_commands.hpp>
#include <iostream>
#include <scan_codes.hpp>
#include <variant>

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
namespace {
std::uint8_t EvCodeToHidCode(std::uint16_t code) {
  switch (code) {
  case KEY_RESERVED:
    return HID_KEY_NONE;
  case KEY_ESC:
    return HID_KEY_ESCAPE;

  case KEY_1:
    return HID_KEY_1;
  case KEY_2:
    return HID_KEY_2;
  case KEY_3:
    return HID_KEY_3;
  case KEY_4:
    return HID_KEY_4;
  case KEY_5:
    return HID_KEY_5;
  case KEY_6:
    return HID_KEY_6;
  case KEY_7:
    return HID_KEY_7;
  case KEY_8:
    return HID_KEY_8;
  case KEY_9:
    return HID_KEY_9;
  case KEY_0:
    return HID_KEY_0;

  case KEY_MINUS:
    return HID_KEY_MINUS;
  case KEY_EQUAL:
    return HID_KEY_EQUAL;
  case KEY_BACKSPACE:
    return HID_KEY_BACKSPACE;
  case KEY_TAB:
    return HID_KEY_TAB;

  case KEY_Q:
    return HID_KEY_Q;
  case KEY_W:
    return HID_KEY_W;
  case KEY_E:
    return HID_KEY_E;
  case KEY_R:
    return HID_KEY_R;
  case KEY_T:
    return HID_KEY_T;
  case KEY_Y:
    return HID_KEY_Y;
  case KEY_U:
    return HID_KEY_U;
  case KEY_I:
    return HID_KEY_I;
  case KEY_O:
    return HID_KEY_O;
  case KEY_P:
    return HID_KEY_P;

  case KEY_LEFTBRACE:
    return HID_KEY_BRACKET_LEFT;
  case KEY_RIGHTBRACE:
    return HID_KEY_BRACKET_RIGHT;
  case KEY_ENTER:
    return HID_KEY_ENTER;
  case KEY_LEFTCTRL:
    return HID_KEY_CONTROL_LEFT;

  case KEY_A:
    return HID_KEY_A;
  case KEY_S:
    return HID_KEY_S;
  case KEY_D:
    return HID_KEY_D;
  case KEY_F:
    return HID_KEY_F;
  case KEY_G:
    return HID_KEY_G;
  case KEY_H:
    return HID_KEY_H;
  case KEY_J:
    return HID_KEY_J;
  case KEY_K:
    return HID_KEY_K;
  case KEY_L:
    return HID_KEY_L;
  case KEY_SEMICOLON:
    return HID_KEY_SEMICOLON;
  case KEY_APOSTROPHE:
    return HID_KEY_APOSTROPHE;

  case KEY_GRAVE:
    return HID_KEY_GRAVE;
  case KEY_LEFTSHIFT:
    return HID_KEY_SHIFT_LEFT;
  case KEY_BACKSLASH:
    return HID_KEY_BACKSLASH;

  case KEY_Z:
    return HID_KEY_Z;
  case KEY_X:
    return HID_KEY_X;
  case KEY_C:
    return HID_KEY_C;
  case KEY_V:
    return HID_KEY_V;
  case KEY_B:
    return HID_KEY_B;
  case KEY_N:
    return HID_KEY_N;
  case KEY_M:
    return HID_KEY_M;
  case KEY_COMMA:
    return HID_KEY_COMMA;
  case KEY_DOT:
    return HID_KEY_PERIOD;
  case KEY_SLASH:
    return HID_KEY_SLASH;
  case KEY_RIGHTSHIFT:

    return HID_KEY_SHIFT_RIGHT;
  case KEY_LEFTALT:
    return HID_KEY_ALT_LEFT;
  case KEY_SPACE:
    return HID_KEY_SPACE;
  case KEY_CAPSLOCK:
    return HID_KEY_CAPS_LOCK;

  case KEY_F1:
    return HID_KEY_F1;
  case KEY_F2:
    return HID_KEY_F2;
  case KEY_F3:
    return HID_KEY_F3;
  case KEY_F4:
    return HID_KEY_F4;
  case KEY_F5:
    return HID_KEY_F5;
  case KEY_F6:
    return HID_KEY_F6;
  case KEY_F7:
    return HID_KEY_F7;
  case KEY_F8:
    return HID_KEY_F8;
  case KEY_F9:
    return HID_KEY_F9;
  case KEY_F10:
    return HID_KEY_F10;

  case KEY_NUMLOCK:
    return HID_KEY_NUM_LOCK;

  case KEY_SCROLLLOCK:
    return HID_KEY_SCROLL_LOCK;
  case KEY_KP7:
    return HID_KEY_KEYPAD_7;
  case KEY_KP8:
    return HID_KEY_KEYPAD_8;
  case KEY_KP9:
    return HID_KEY_KEYPAD_9;
  case KEY_KPMINUS:
    return HID_KEY_KEYPAD_SUBTRACT;
  case KEY_KP4:
    return HID_KEY_KEYPAD_4;
  case KEY_KP5:
    return HID_KEY_KEYPAD_5;
  case KEY_KP6:
    return HID_KEY_KEYPAD_6;
  case KEY_KPPLUS:
    return HID_KEY_KEYPAD_ADD;
  case KEY_KP1:
    return HID_KEY_KEYPAD_1;
  case KEY_KP2:
    return HID_KEY_KEYPAD_2;
  case KEY_KP3:
    return HID_KEY_KEYPAD_3;
  case KEY_KP0:
    return HID_KEY_KEYPAD_0;
  case KEY_KPDOT:
    return HID_KEY_KEYPAD_DECIMAL;

  case KEY_F11:
    return HID_KEY_F11;
  case KEY_F12:
    return HID_KEY_F12;
  case KEY_KPENTER:
    return HID_KEY_KEYPAD_ENTER;
  case KEY_RIGHTCTRL:
    return HID_KEY_CONTROL_RIGHT;
  case KEY_KPSLASH:
    return HID_KEY_KEYPAD_DIVIDE;
  case KEY_RIGHTALT:
    return HID_KEY_ALT_RIGHT;
  case KEY_HOME:
    return HID_KEY_HOME;
  case KEY_UP:
    return HID_KEY_ARROW_UP;
  case KEY_PAGEUP:
    return HID_KEY_PAGE_UP;
  case KEY_LEFT:
    return HID_KEY_ARROW_LEFT;
  case KEY_RIGHT:
    return HID_KEY_ARROW_RIGHT;
  case KEY_END:
    return HID_KEY_END;
  case KEY_DOWN:
    return HID_KEY_ARROW_DOWN;
  case KEY_PAGEDOWN:
    return HID_KEY_PAGE_DOWN;
  case KEY_INSERT:
    return HID_KEY_INSERT;
  case KEY_DELETE:
    return HID_KEY_DELETE;
  case KEY_MUTE:
    return HID_KEY_MUTE;
  case KEY_VOLUMEDOWN:
    return HID_KEY_VOLUME_DOWN;
  case KEY_VOLUMEUP:
    return HID_KEY_VOLUME_UP;
  case KEY_POWER:
    return HID_KEY_POWER;
  case KEY_KPEQUAL:
    return HID_KEY_KEYPAD_EQUAL;
  case KEY_KPPLUSMINUS:
    return HID_KEY_KEYPAD_PLUS_MINUS;
  case KEY_PAUSE:
    return HID_KEY_PAUSE;
  case KEY_KPCOMMA:
    return HID_KEY_KEYPAD_COMMA;
  case KEY_LEFTMETA:
    return HID_KEY_GUI_LEFT;
  case KEY_RIGHTMETA:
    return HID_KEY_GUI_RIGHT;
  case BTN_LEFT:
    return 0;
  case BTN_RIGHT:
    return 1;
  case BTN_MIDDLE:
    return 2;
  case BTN_SIDE:
    return 3;
  case BTN_EXTRA:
    return 4;
  default:
    return HID_KEY_NONE;
  }
}
} // namespace

namespace HidStructs {

bool HidReport::process_key(const KeyState &state) {
  if (state.keycode < HID_KEY_KEYPAD_HEXADECIMAL) {
    uint8_t bit_index = state.keycode % 8;
    uint8_t array_index = state.keycode / 8;
    uint8_t prev = nkro_.keybits[array_index];
    if (state.pressed) {
      nkro_.keybits[array_index] |= 1 << bit_index;
    } else {
      nkro_.keybits[array_index] &= ~(1 << bit_index);
    }
    return prev != nkro_.keybits[array_index];
  } else if (state.keycode >= HID_KEY_CONTROL_LEFT &&
             state.keycode <= HID_KEY_GUI_RIGHT) {
    uint8_t bit_index = state.keycode - HID_KEY_CONTROL_LEFT;
    uint8_t prev = nkro_.modifier;
    if (state.pressed) {
      nkro_.modifier |= 1 << bit_index;
    } else {
      nkro_.modifier &= ~(1 << bit_index);
    }
    return prev != nkro_.modifier;
  } else {
    return false;
  }
}

bool HidReport::process_mouse(const MouseState &state) {
  if (state.x != 0 || state.y != 0 || state.wheel != 0) {
    mouse_.x += state.x;
    mouse_.y += state.y;
    mouse_.wheel += state.wheel;
    return true;
  } else {
    return false;
  }
}

bool HidReport::process_buttons(const MouseButtonState &state) {
  std::uint8_t prev = mouse_.buttons;
  if (state.pressed) {
    mouse_.buttons |= 1 << state.button_code;
  } else {
    mouse_.buttons &= ~(1 << state.button_code);
  }
  return mouse_.buttons != prev;
}

std::uint8_t HidReport::SerializeNKRO(uint8_t *buffer) {
  buffer[0] = static_cast<uint8_t>(ReportType::REPORT_ID_NKRO);
  buffer[1] = NKRO_BYTE_COUNT + 1;
  buffer[2] = nkro_.modifier;
  for (int i = 0; i < NKRO_BYTE_COUNT; i++) {
    buffer[i + 3] = nkro_.keybits[i];
  }
  return NKRO_BYTE_COUNT + 3;
}

std::uint8_t HidReport::SerializeMouse(uint8_t *buffer) {
  buffer[0] = static_cast<uint8_t>(ReportType::REPORT_ID_MOUSE);
  buffer[1] = 8;
  buffer[2] = mouse_.buttons;
  buffer[3] = mouse_.x & 0xFF;
  buffer[4] = (mouse_.x >> 8) & 0xFF;
  buffer[5] = mouse_.y & 0xFF;
  buffer[6] = (mouse_.y >> 8) & 0xFF;
  buffer[7] = mouse_.wheel & 0xFF;
  buffer[8] = (mouse_.wheel >> 8) & 0xFF;
  buffer[9] = 0;
  return 10;
}

HidStructs::HidState KeyboardPollDevice::HandleEvEvent(const input_event &evt) {
  HidState state = std::monostate();
  if (evt.type == EV_KEY) {
    HidStructs::KeyState key_state;
    key_state.keycode = EvCodeToHidCode(evt.code);
    if (evt.value == 1 || evt.value == 0) {
      key_state.pressed = evt.value == 1;
      state = key_state;
    }
  }
  return state;
}

HidStructs::HidState MousePollDevice::HandleEvEvent(const input_event &evt) {
  switch (evt.type) {
  case EV_REL: {
    switch (evt.code) {
    case REL_X:
      state_.x += evt.value;
      break;
    case REL_Y:
      state_.y += evt.value;
      break;
    case REL_WHEEL:
      state_.wheel += evt.value;
      break;
    default:
      break;
    }
  }
  case EV_SYN: {
    if (state_.x != 0 || state_.y != 0 || state_.wheel != 0) {
      return state_;
    }
    break;
  }
  case EV_KEY: {
    if (evt.code >= BTN_LEFT && evt.code <= BTN_EXTRA) {
      HidStructs::MouseButtonState state;
      state.button_code = EvCodeToHidCode(evt.code);
      if (evt.value == 1 || evt.value == 0) {
        state.pressed = evt.value == 1;
        return state;
      }
    }
    break;
  }
  default:
    break;
  }
  return std::monostate();
}

std::uint8_t HidReport::GenerateReport(const HidStructs::HidState &evt,
                                       uint8_t *buffer) {
  std::uint8_t res = std::visit(
      overloaded{
          [](const std::monostate &_arg) { return static_cast<uint8_t>(0); },
          [this, buffer](const HidStructs::KeyState &key) {
            if (process_key(key)) {
              uint8_t res = SerializeNKRO(buffer);
              return res;
            } else {
              return static_cast<uint8_t>(0);
            };
          },
          [this, buffer](const HidStructs::MouseState &mouse) {
            if (process_mouse(mouse)) {
              uint8_t res = SerializeMouse(buffer);
              mouse_.x = 0;
              mouse_.y = 0;
              mouse_.wheel = 0;
              return res;
            } else {
              return static_cast<uint8_t>(0);
            };
          },
          [this, buffer](const HidStructs::MouseButtonState &buttons) {
            if (process_buttons(buttons)) {
              uint8_t res = SerializeMouse(buffer);
              return res;
            } else {
              return static_cast<uint8_t>(0);
            };
          },
      },
      evt);
  return res;
}

} // namespace HidStructs
