// test/test_emulate.c
#include "emulate.h"
#include "report_types.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define KEYBOARD_A 0x04
#define KEYBOARD_F 0x09
#define KEYBOARD_LSFT 0xE1

void setUp(void) {}

void tearDown(void) {}

void test_simple_keypress_and_release(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = keyboard_command(KEYBOARD_A, PRESSED);

  // Verify that a report was actually generated as pressing A should be a state
  // change
  bool res = generate_report(&state, &report, command);
  TEST_ASSERT_TRUE(res);

  // Verify that the report structure is correct
  TEST_ASSERT_EQUAL(REPORT_ID_NKRO, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_report_nkro_t), report.len);

  // Verify that the 4th bit is set of the nkro report is set
  hid_report_nkro_t expected_nkro;
  expected_nkro.modifier = 0x0;
  memset(expected_nkro.keybits, 0, sizeof(expected_nkro.keybits));
  expected_nkro.keybits[0] = 1 << 4;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);

  // Verify that a report was actually generated as releasing A should be a
  // state change
  command.key.state = RELEASED;
  res = generate_report(&state, &report, command);
  TEST_ASSERT_TRUE(res);

  // Verify that the report structure is correct
  TEST_ASSERT_EQUAL(report.report_id, REPORT_ID_NKRO);
  TEST_ASSERT_EQUAL(report.len, sizeof(hid_report_nkro_t));

  // Verify that the 4th bit was cleared and nothing else was modified
  expected_nkro.keybits[0] = 0;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);
}

void test_repeated_keypresses(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = keyboard_command(KEYBOARD_A, PRESSED);

  // Verify that a report was actually generated as pressing A should be a state
  // change
  bool res = generate_report(&state, &report, command);
  TEST_ASSERT_TRUE(res);

  // Verify that sending the same command makes no change to the state
  res = generate_report(&state, &report, command);
  TEST_ASSERT_FALSE(res);

  // Verify that the internal state stays the same
  hid_report_nkro_t expected_nkro;
  expected_nkro.modifier = 0x0;
  memset(expected_nkro.keybits, 0, sizeof(expected_nkro.keybits));
  expected_nkro.keybits[0] = 1 << 4;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits, state.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, state.nkro.modifier);

  // Verify that duplicate releases are handled properly as well
  command.key.state = RELEASED;
  res = generate_report(&state, &report, command);
  TEST_ASSERT_TRUE(res);
  res = generate_report(&state, &report, command);
  TEST_ASSERT_FALSE(res);

  expected_nkro.keybits[0] = 0;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits, state.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, state.nkro.modifier);
}

void test_multiple_keypresses(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = keyboard_command(KEYBOARD_A, PRESSED);
  hid_command_t command2 = keyboard_command(KEYBOARD_F, PRESSED);

  // Verify that a report was actually generated as pressing A should be a state
  // change
  bool res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_NKRO, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_report_nkro_t), report.len);
  hid_report_nkro_t expected_nkro;
  expected_nkro.modifier = 0x0;
  memset(expected_nkro.keybits, 0, sizeof(expected_nkro.keybits));
  expected_nkro.keybits[KEYBOARD_A / 8] = 1 << (KEYBOARD_A % 8);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);

  res = generate_report(&state, &report, command2);
  TEST_ASSERT(res);
  // Both A and F bits should be set
  TEST_ASSERT_EQUAL(REPORT_ID_NKRO, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_report_nkro_t), report.len);
  expected_nkro.keybits[KEYBOARD_F / 8] = 1 << (KEYBOARD_F % 8);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);
}

void test_modifier_keypress(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = keyboard_command(KEYBOARD_LSFT, PRESSED);

  bool res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_NKRO, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_report_nkro_t), report.len);
  hid_report_nkro_t expected_nkro;
  expected_nkro.modifier = 1 << 1;
  memset(expected_nkro.keybits, 0, sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);

  command.key.state = RELEASED;
  res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_NKRO, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_report_nkro_t), report.len);
  expected_nkro.modifier = 0;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_nkro.keybits,
                                report.payload.nkro.keybits,
                                sizeof(expected_nkro.keybits));
  TEST_ASSERT_EQUAL(expected_nkro.modifier, report.payload.nkro.modifier);
}

void test_mouse_movement(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = mouse_command(-5, 2);

  bool res = generate_report(&state, &report, command);

  // Assert that we have an actual report and its the correct structure
  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_MOUSE, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_mouse_report_t), report.len);

  // x and y are the only variables that should be modified here
  hid_mouse_report_t expected_mouse;
  expected_mouse.buttons = 0;
  expected_mouse.pan = 0;
  expected_mouse.wheel = 0;
  expected_mouse.x = -5;
  expected_mouse.y = 2;
  TEST_ASSERT_EQUAL(expected_mouse.buttons, report.payload.mouse.buttons);
  TEST_ASSERT_EQUAL(expected_mouse.pan, report.payload.mouse.pan);
  TEST_ASSERT_EQUAL(expected_mouse.wheel, report.payload.mouse.wheel);
  TEST_ASSERT_EQUAL(expected_mouse.x, report.payload.mouse.x);
  TEST_ASSERT_EQUAL(expected_mouse.y, report.payload.mouse.y);

  // Verify that sending the same command still results in a report that's
  // generated
  res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_MOUSE, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_mouse_report_t), report.len);

  // x and y are the only variables that should be modified here
  TEST_ASSERT_EQUAL(expected_mouse.buttons, report.payload.mouse.buttons);
  TEST_ASSERT_EQUAL(expected_mouse.pan, report.payload.mouse.pan);
  TEST_ASSERT_EQUAL(expected_mouse.wheel, report.payload.mouse.wheel);
  TEST_ASSERT_EQUAL(expected_mouse.x, report.payload.mouse.x);
  TEST_ASSERT_EQUAL(expected_mouse.y, report.payload.mouse.y);

  // Verify sending a zeroed delata move results in no report
  command.mouse.x = 0;
  command.mouse.y = 0;
  res = generate_report(&state, &report, command);
  TEST_ASSERT_FALSE(res);
}

void test_mouse_buttons(void) {
  hid_state_t state;
  memset(&state, 0, sizeof(state));
  hid_generic_report_t report;
  hid_command_t command = button_command(1, PRESSED);

  bool res = generate_report(&state, &report, command);

  // Assert that we have an actual report and its the correct structure
  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_MOUSE, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_mouse_report_t), report.len);

  // Buttons is the only variable that should change
  hid_mouse_report_t expected_mouse;
  expected_mouse.buttons = 1;
  expected_mouse.pan = 0;
  expected_mouse.wheel = 0;
  expected_mouse.x = 0;
  expected_mouse.y = 0;
  TEST_ASSERT_EQUAL(expected_mouse.buttons, report.payload.mouse.buttons);
  TEST_ASSERT_EQUAL(expected_mouse.pan, report.payload.mouse.pan);
  TEST_ASSERT_EQUAL(expected_mouse.wheel, report.payload.mouse.wheel);
  TEST_ASSERT_EQUAL(expected_mouse.x, report.payload.mouse.x);
  TEST_ASSERT_EQUAL(expected_mouse.y, report.payload.mouse.y);

  // Verify that sending mouse delta command doesn't affect button state
  command = mouse_command(5, 2);
  res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_MOUSE, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_mouse_report_t), report.len);

  expected_mouse.x = 5;
  expected_mouse.y = 2;
  TEST_ASSERT_EQUAL(expected_mouse.buttons, report.payload.mouse.buttons);
  TEST_ASSERT_EQUAL(expected_mouse.pan, report.payload.mouse.pan);
  TEST_ASSERT_EQUAL(expected_mouse.wheel, report.payload.mouse.wheel);
  TEST_ASSERT_EQUAL(expected_mouse.x, report.payload.mouse.x);
  TEST_ASSERT_EQUAL(expected_mouse.y, report.payload.mouse.y);

  // Verify that mouse delta state doesn't linger in future button presses
  command = button_command(1 << 1, PRESSED);
  res = generate_report(&state, &report, command);

  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(REPORT_ID_MOUSE, report.report_id);
  TEST_ASSERT_EQUAL(sizeof(hid_mouse_report_t), report.len);

  expected_mouse.buttons = 0b11;
  expected_mouse.x = 0;
  expected_mouse.y = 0;
  TEST_ASSERT_EQUAL(expected_mouse.buttons, report.payload.mouse.buttons);
  TEST_ASSERT_EQUAL(expected_mouse.pan, report.payload.mouse.pan);
  TEST_ASSERT_EQUAL(expected_mouse.wheel, report.payload.mouse.wheel);
  TEST_ASSERT_EQUAL(expected_mouse.x, report.payload.mouse.x);
  TEST_ASSERT_EQUAL(expected_mouse.y, report.payload.mouse.y);
}

void test_deserialization(void) {
  // Test keyboard deserialization
  hid_command_t command;
  uint8_t buffer[sizeof(hid_command_t)];
  buffer[0] = KEYBOARD_COMMAND;
  buffer[1] = KEYBOARD_A;
  buffer[2] = PRESSED;
  hid_command_t actual_command = keyboard_command(KEYBOARD_A, PRESSED);
  bool res = deserialize_command(buffer, 3, &command);
  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(actual_command.type, command.type);
  TEST_ASSERT_EQUAL(actual_command.key.keycode, command.key.keycode);
  TEST_ASSERT_EQUAL(actual_command.key.state, command.key.state);

  // Test mouse deserialization
  buffer[0] = MOUSE_COMMAND;
  buffer[1] = -5;
  buffer[2] = 2;
  actual_command = mouse_command(-5, 2);
  res = deserialize_command(buffer, 3, &command);
  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(actual_command.type, command.type);
  TEST_ASSERT_EQUAL(actual_command.key.keycode, command.key.keycode);
  TEST_ASSERT_EQUAL(actual_command.key.state, command.key.state);

  // Test mouse button  deserialization
  buffer[0] = MOUSE_BUTTON_COMMAND;
  buffer[1] = 3;
  buffer[2] = PRESSED;
  actual_command = button_command(3, PRESSED);
  res = deserialize_command(buffer, 3, &command);
  TEST_ASSERT(res);
  TEST_ASSERT_EQUAL(actual_command.type, command.type);
  TEST_ASSERT_EQUAL(actual_command.key.keycode, command.key.keycode);
  TEST_ASSERT_EQUAL(actual_command.key.state, command.key.state);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_simple_keypress_and_release);
  RUN_TEST(test_repeated_keypresses);
  RUN_TEST(test_multiple_keypresses);
  RUN_TEST(test_modifier_keypress);
  RUN_TEST(test_mouse_movement);
  RUN_TEST(test_mouse_buttons);
  RUN_TEST(test_deserialization);
  return UNITY_END();
}
