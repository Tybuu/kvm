#pragma once

#include <cstdint>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <variant>
#include <vector>

namespace HidStructs {
constexpr std::size_t NKRO_KEY_COUNT = 248;
constexpr std::size_t NKRO_BYTE_COUNT = (NKRO_KEY_COUNT / 8);
constexpr std::size_t BUFFER_SIZE = 66;

typedef struct {
  std::uint8_t buttons;
  std::int16_t x;
  std::int16_t y;
  std::int16_t wheel;
  std::int8_t pan;
} HidMouseReport;

enum class ReportType : std::uint8_t {
  REPORT_ID_NKRO = 1,
  REPORT_ID_MOUSE = 2,
  REPORT_ID_INOUT = 3,
};

typedef struct {
  std::uint8_t modifier;
  std::uint8_t keybits[NKRO_BYTE_COUNT];
} HidReportNKRO;

typedef struct {
  std::uint8_t report_id;
  std::uint8_t len;
  union {
    HidReportNKRO nkro;
    HidMouseReport mouse;
    std::uint8_t vendor[64];
  } payload;
} HidGenericReport;

typedef struct {
  std::int16_t x;
  std::int16_t y;
  std::int16_t wheel;
} MouseState;

typedef struct {
  std::uint8_t keycode;
  bool pressed;
} KeyState;

typedef struct {
  std::uint8_t button_code;
  bool pressed;
} MouseButtonState;

using HidState =
    std::variant<std::monostate, KeyState, MouseState, MouseButtonState>;

class HidReport {
private:
  HidReportNKRO nkro_;
  HidMouseReport mouse_;
  HidState state_;

  // Modifies state_ and returns true if
  // state_  is ready to be processed
  bool process_input(const input_event &evt);

  // Processes the key state that potentially modifies
  // the internal NKRO report. If the NKRO report is modified,
  // returns true, false otherwise
  bool process_key(const KeyState &state);

  // Processes the mouse state that potentially modifies
  // the internal mouse report. If the mouse report is modified,
  // returns true, false otherwise
  bool process_mouse(const MouseState &state);

  // Processes the mouse button state that potentially modifies
  // the internal mouse report. If the mouse report is modified,
  // returns true, false otherwise
  bool process_buttons(const MouseButtonState &state);

public:
  HidReport() : nkro_{}, mouse_{}, state_{} {};

  // Takes in an input_event and attempts to generate an Hid Report
  // if the state changes. If a report is generated, the report
  // will be serialized into the buffer and the number of bytes written
  // is returned. Otherwise, the function will return a 0
  std::uint8_t GenerateReport(const input_event &evt, std::uint8_t *buffer);

  // Serializes the current NKRO report state into the buffer
  // and returns the number of bytes written
  std::uint8_t SerializeNKRO(std::uint8_t *buffer);

  // Serializes the current Mouse report state into the buffer
  // and returns the number of bytes written
  std::uint8_t SerializeMouse(std::uint8_t *buffer);
};
} // namespace HidStructs
