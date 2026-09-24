#ifndef REPORT_TYPES_H
#define REPORT_TYPES_H

#ifndef HID_KEY_CONTROL_LEFT
#define HID_KEY_CONTROL_LEFT 0xE0
#endif
#ifndef HID_KEY_CONTROL_RIGHT
#define HID_KEY_CONTROL_RIGHT 0xE4
#endif
#ifndef HID_KEY_GUI_RIGHT
#define HID_KEY_GUI_RIGHT 0xE7
#endif
#ifndef HID_KEY_KEYPAD_HEXADECIMAL
#define HID_KEY_KEYPAD_HEXADECIMAL 0xDD
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#ifndef _TUSB_HID_H_
typedef struct __attribute__((packed)) {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t wheel;
  int8_t pan;
} hid_mouse_report_t;
#endif
#define NKRO_KEY_COUNT 248
#define NKRO_BYTE_COUNT (NKRO_KEY_COUNT / 8)

enum {
  REPORT_ID_NKRO = 1,
  REPORT_ID_MOUSE = 2,
  REPORT_ID_INOUT = 3,
};
typedef struct __attribute__((packed)) {
  uint8_t modifier;
  uint8_t keybits[NKRO_BYTE_COUNT];
} hid_report_nkro_t;

typedef struct {
  uint8_t report_id;
  uint8_t len;
  union {
    hid_report_nkro_t nkro;
    hid_mouse_report_t mouse;
    uint8_t vendor[64];
  } payload;
} hid_generic_report_t;

static inline void set_generic_nkro(hid_generic_report_t *rep,
                                    hid_report_nkro_t *nkro) {
  // memcpy(&rep->payload.nkro.keybits, &nkro->keybits, sizeof(nkro->keybits));
  // rep->payload.nkro.modifier = nkro->modifier;
  rep->payload.nkro = *nkro;
  rep->report_id = REPORT_ID_NKRO;
  rep->len = sizeof(hid_report_nkro_t);
}

static inline void set_generic_mouse(hid_generic_report_t *rep,
                                     hid_mouse_report_t *mouse) {
  rep->payload.mouse = *mouse;
  rep->report_id = REPORT_ID_MOUSE;
  rep->len = sizeof(hid_mouse_report_t);
}

static inline void set_generic_inout(hid_generic_report_t *rep,
                                     uint8_t *vendor) {
  for (int i = 0; i < 64; i++) {
    rep->payload.vendor[i] = vendor[i];
  }
  rep->report_id = REPORT_ID_INOUT;
  rep->len = sizeof(rep->payload.vendor);
}

static inline void nkro_clear(hid_report_nkro_t *report) {
  memset(report, 0, sizeof(hid_report_nkro_t));
}

// Presses the passed in key from the report. Returns true if the report
// changed
static inline bool nkro_press_key(hid_report_nkro_t *report, uint8_t key) {
  // Keys Modifier
  if (key < HID_KEY_KEYPAD_HEXADECIMAL) {
    uint8_t bit_index = key % 8;
    uint8_t array_index = key / 8;
    uint8_t prev = report->keybits[array_index];
    report->keybits[array_index] |= 1 << bit_index;
    return prev != report->keybits[array_index];
  } else if (key >= HID_KEY_CONTROL_LEFT && key <= HID_KEY_GUI_RIGHT) {
    uint8_t bit_index = key - HID_KEY_CONTROL_LEFT;
    uint8_t prev = report->modifier;
    report->modifier |= 1 << bit_index;
    return prev != report->modifier;
  } else {
    return false;
  }
}

// Releases the passed in key from the report. Returns true if the report
// changed
static inline bool nkro_release_key(hid_report_nkro_t *report, uint8_t key) {
  // Keys Modifier
  if (key <= HID_KEY_KEYPAD_HEXADECIMAL) {
    uint8_t bit_index = key % 8;
    uint8_t array_index = key / 8;
    uint8_t prev = report->keybits[array_index];
    report->keybits[array_index] &= ~(1 << bit_index);
    return prev != report->keybits[array_index];
  } else if (key >= HID_KEY_CONTROL_LEFT && key <= HID_KEY_GUI_RIGHT) {
    uint8_t bit_index = key - HID_KEY_CONTROL_LEFT;
    uint8_t prev = report->modifier;
    report->modifier &= ~(1 << bit_index);
    return prev != report->modifier;
  } else {
    return false;
  }
}

static inline void mouse_clear(hid_mouse_report_t *report) {
  memset(report, 0, sizeof(hid_mouse_report_t));
}

static inline void mouse_clear_delta(hid_mouse_report_t *report) {
  report->x = 0;
  report->y = 0;
  report->wheel = 0;
  report->pan = 0;
}

static inline bool mouse_press_button(hid_mouse_report_t *report, uint8_t key) {
  uint8_t prev = report->buttons;
  report->buttons |= key;
  return prev != report->buttons;
}

static inline bool mouse_release_button(hid_mouse_report_t *report,
                                        uint8_t key) {
  uint8_t prev = report->buttons;
  report->buttons &= ~key;
  return prev != report->buttons;
}

static inline void mouse_move(hid_mouse_report_t *report, int8_t x, int8_t y) {
  report->x += x;
  report->y += y;
}
#endif /* REPORT_TYPES_H */
