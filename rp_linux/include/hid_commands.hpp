#pragma once

#include <cstddef>
#include <cstdint>

namespace HidStructs {
constexpr std::size_t NKRO_KEY_COUNT = 248;
constexpr std::size_t NKRO_BYTE_COUNT = (NKRO_KEY_COUNT / 8);
constexpr std::size_t BUFFER_SIZE = 66;

typedef struct __attribute__((packed)) {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t wheel;
  int8_t pan;
} hid_mouse_report_t;

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

class HidReport {
private:
  hid_report_nkro_t keyboard;
  hid_mouse_report_t mouse;

public:
  HidReport();

  // TODO: Add commnad to determine if report should be generated
  bool generate_report(uint8_t *buffer);

  void get_nkro(uint8_t *buffer);

  void get_mouse(uint8_t *buffer);
};
} // namespace HidStructs
