/* Copyright 2023 Cipulot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ec_60.h"

#include <eeprom.h>
#include <send_string.h>
#include <stdint.h>

#include "ec_60/config.h"
#include "ec_switch_matrix.h"

// Declaring enums for VIA config menu
enum via_apc_enums {
  // clang-format off
  id_ec_actuation_mode = 1,
  id_ec_mode_0_actuation_threshold,
  id_ec_mode_0_release_threshold,
  id_ec_mode_1_initial_deadzone_offset,
  id_ec_mode_1_actuation_sensitivity,
  id_ec_mode_1_release_sensitivity,
  id_ec_bottoming_calibration,
  id_ec_show_calibration_data
  // clang-format on
};

#define NO_KEY_THRESHOLD 0x60

static deferred_token show_calibration_data_token;  // defer_exec token
static bool ec_config_initialized;
static int ec_config_error;

#ifdef ENABLE_CALIBRATED_BOTTOMING_READING
const uint16_t PROGMEM caliblated_bottming_reading[MATRIX_ROWS][MATRIX_COLS] = {
  {0x025a, 0x02a2, 0x025d, 0x02f9, 0x0256, 0x025b, 0x0201, 0x022c, 0x023b, 0x01eb, 0x0240, 0x0229, 0x0290, 0x018c,
   0x0218},
  {0x0280, 0x02c8, 0x030c, 0x036a, 0x02d8, 0x030e, 0x02b7, 0x02b4, 0x0311, 0x02cd, 0x0284, 0x0281, 0x0229, 0x01d6,
   0x03ff},
  {0x01c3, 0x0310, 0x0317, 0x0336, 0x02bb, 0x031a, 0x02c7, 0x030d, 0x02fd, 0x0277, 0x028e, 0x02eb, 0x03ff, 0x027b,
   0x03ff},
  {0x01eb, 0x03ff, 0x026e, 0x02df, 0x0341, 0x02e3, 0x0297, 0x0340, 0x033a, 0x02ad, 0x02bb, 0x02b2, 0x03ff, 0x0177,
   0x01da},
  {0x0208, 0x01ef, 0x01e4, 0x03ff, 0x03ff, 0x03ff, 0x01dd, 0x03ff, 0x03ff, 0x03ff, 0x0158, 0x0192, 0x01c7, 0x03ff,
   0x03ff}};
#endif

static int is_eeprom_ec_config_valid(void);
static void defer_show_calibration_data(uint32_t delay_ms);
static void ec_config_rescale(void);
static void ec_config_set_value(uint8_t *data);
static void ec_config_get_value(uint8_t *data);

// QMK hook functions
// -----------------------------------------------------------------------------------

void eeconfig_init_user(void) {
  // Default values
  eeprom_ec_config.actuation_mode = DEFAULT_ACTUATION_MODE;
  eeprom_ec_config.mode_0_actuation_threshold = DEFAULT_MODE_0_ACTUATION_LEVEL;
  eeprom_ec_config.mode_0_release_threshold = DEFAULT_MODE_0_RELEASE_LEVEL;
  eeprom_ec_config.mode_1_initial_deadzone_offset = DEFAULT_MODE_1_INITIAL_DEADZONE_OFFSET;
  eeprom_ec_config.mode_1_actuation_sensitivity = DEFAULT_MODE_1_ACTUATION_SENSITIVITY;
  eeprom_ec_config.mode_1_release_sensitivity = DEFAULT_MODE_1_RELEASE_SENSITIVITY;

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      // I don't want to lose calibration data for each update firware
#ifdef ENABLE_CALIBRATED_BOTTOMING_READING
      eeprom_ec_config.bottoming_reading[row][col] = pgm_read_word(&caliblated_bottming_reading[row][col]);
#else
      eeprom_ec_config.bottoming_reading[row][col] = DEFAULT_BOTTOMING_READING;
#endif
    }
  }
  // Write default value to EEPROM now
  eeprom_update_block(&eeprom_ec_config, (void *)VIA_EEPROM_CUSTOM_CONFIG_USER_ADDR, sizeof(eeprom_ec_config_t));
  ec_config_initialized = true;
}

// On Keyboard startup
void keyboard_post_init_user(void) {
  // Read custom menu variables from memory
  eeprom_read_block(&eeprom_ec_config, (void *)VIA_EEPROM_CUSTOM_CONFIG_USER_ADDR, sizeof(eeprom_ec_config_t));

  ec_config_error = is_eeprom_ec_config_valid();
  if (ec_config_error != 0) {
    eeconfig_init_user();
  }

  // Set runtime values to EEPROM values
  ec_config.actuation_mode = eeprom_ec_config.actuation_mode;
  ec_config.mode_0_actuation_threshold = eeprom_ec_config.mode_0_actuation_threshold;
  ec_config.mode_0_release_threshold = eeprom_ec_config.mode_0_release_threshold;
  ec_config.mode_1_initial_deadzone_offset = eeprom_ec_config.mode_1_initial_deadzone_offset;
  ec_config.mode_1_actuation_sensitivity = eeprom_ec_config.mode_1_actuation_sensitivity;
  ec_config.mode_1_release_sensitivity = eeprom_ec_config.mode_1_release_sensitivity;
  ec_config.bottoming_calibration = false;
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.bottoming_calibration_starter[row][col] = true;
      ec_config.bottoming_reading[row][col] = eeprom_ec_config.bottoming_reading[row][col];
    }
  }
  ec_config_rescale();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case EC_DBG:
      // release after 1 scond.
      if (!record->event.pressed) {
        defer_show_calibration_data(1000UL);
        return false;
      }
  }
  return true;
}

// common lib hook functions
// -----------------------------------------------------------------------------------

bool via_custom_value_command_user(uint8_t *data, uint8_t length) {
  // data = [ command_id, channel_id, value_id, value_data ]
  uint8_t *command_id = &(data[0]);
  uint8_t *channel_id = &(data[1]);
  uint8_t *value_id_and_data = &(data[2]);

  if (*channel_id == VIA_EC_CUSTOM_CHANNEL_ID) {
    switch (*command_id) {
      case id_custom_set_value: {
        ec_config_set_value(value_id_and_data);
        return false;
      }
      case id_custom_get_value: {
        ec_config_get_value(value_id_and_data);
        return false;
      }
    }
  }
  return true;
}

// static routines
// -----------------------------------------------------------------------------------

// TODO more strict validation
static int is_eeprom_ec_config_valid(void) {
  if (eeprom_ec_config.actuation_mode > 1) return false;
  if (eeprom_ec_config.mode_0_actuation_threshold > DEFAULT_EXTREMUM || eeprom_ec_config.mode_0_actuation_threshold < 1)
    return -1;
  if (eeprom_ec_config.mode_0_release_threshold > DEFAULT_EXTREMUM || eeprom_ec_config.mode_0_release_threshold < 1)
    return -2;
  if (eeprom_ec_config.mode_1_initial_deadzone_offset > DEFAULT_EXTREMUM ||
      eeprom_ec_config.mode_1_initial_deadzone_offset < 1)
    return -3;

  if (eeprom_ec_config.mode_1_actuation_sensitivity == 0) return -4;
  if (eeprom_ec_config.mode_1_release_sensitivity == 0) return -5;

  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      // ADC 12bit EXTRENUM 10bit
      if (eeprom_ec_config.bottoming_reading[row][col] <= ec_config.noise_floor[row][col] ||
          eeprom_ec_config.bottoming_reading[row][col] > 0xfff)
        return -7;
    }
  }
  return 0;
}

static void ec_config_rescale_mode_0_actuation_threshold(void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.rescaled_mode_0_actuation_threshold[row][col] =
        rescale(ec_config.mode_0_actuation_threshold, 0, 1023, ec_config.noise_floor[row][col],
                eeprom_ec_config.bottoming_reading[row][col]);
    }
  }
}

static void ec_config_rescale_mode_0_release_threshold(void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.rescaled_mode_0_release_threshold[row][col] =
        rescale(ec_config.mode_0_release_threshold, 0, 1023, ec_config.noise_floor[row][col],
                eeprom_ec_config.bottoming_reading[row][col]);
    }
  }
}

static void ec_config_rescale_mode_1_initial_deadzone_offset(void) {
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.rescaled_mode_1_initial_deadzone_offset[row][col] =
        rescale(ec_config.mode_1_initial_deadzone_offset, 0, 1023, ec_config.noise_floor[row][col],
                eeprom_ec_config.bottoming_reading[row][col]);
    }
  }
}

static void ec_config_rescale(void) {
  ec_config_rescale_mode_0_actuation_threshold();
  ec_config_rescale_mode_0_release_threshold();
  ec_config_rescale_mode_1_initial_deadzone_offset();
}

// Handle the data received by the keyboard from the VIA menus
static void ec_config_set_value(uint8_t *data) {
  // data = [ value_id, value_data ]
  uint8_t *value_id = &(data[0]);
  uint8_t *value_data = &(data[1]);

  switch (*value_id) {
    case id_ec_actuation_mode:
      // dropdown
      ec_config.actuation_mode = value_data[0];
      eeprom_ec_config.actuation_mode = ec_config.actuation_mode;
      eeprom_update_byte((void *)EC_CONFIG_ACTUATION_MODE, eeprom_ec_config.actuation_mode);
      break;
    case id_ec_mode_0_actuation_threshold:
      // range
      ec_config.mode_0_actuation_threshold = (value_data[0] << 8) + value_data[1];
      eeprom_ec_config.mode_0_actuation_threshold = ec_config.mode_0_actuation_threshold;
      ec_config_rescale_mode_0_actuation_threshold();
      defer_eeprom_update_word(VIA_EC_CUSTOM_CHANNEL_ID, id_ec_mode_0_actuation_threshold,
                               (void *)EC_CONFIG_MODE_0_ACTUATION_THRESHOLD,
                               eeprom_ec_config.mode_0_actuation_threshold);
      break;
    case id_ec_mode_0_release_threshold:
      // range
      ec_config.mode_0_release_threshold = (value_data[0] << 8) + value_data[1];
      eeprom_ec_config.mode_0_release_threshold = ec_config.mode_0_release_threshold;
      ec_config_rescale_mode_0_release_threshold();
      defer_eeprom_update_word(VIA_EC_CUSTOM_CHANNEL_ID, id_ec_mode_0_release_threshold,
                               (void *)EC_CONFIG_MODE_0_RELEASE_THRESHOLD, eeprom_ec_config.mode_0_release_threshold);
      break;
    case id_ec_mode_1_initial_deadzone_offset:
      // range
      ec_config.mode_1_initial_deadzone_offset = (value_data[0] << 8) + value_data[1];
      eeprom_ec_config.mode_1_initial_deadzone_offset = ec_config.mode_1_initial_deadzone_offset;
      ec_config_rescale_mode_1_initial_deadzone_offset();
      defer_eeprom_update_word(VIA_EC_CUSTOM_CHANNEL_ID, id_ec_mode_1_initial_deadzone_offset,
                               (void *)EC_CONFIG_MODE_1_INITIAL_DEADZONE_OFFSET,
                               eeprom_ec_config.mode_1_initial_deadzone_offset);
      break;
    case id_ec_mode_1_actuation_sensitivity:
      // range
      ec_config.mode_1_actuation_sensitivity = value_data[0];
      eeprom_ec_config.mode_1_actuation_sensitivity = ec_config.mode_1_actuation_sensitivity;
      defer_eeprom_update_byte(VIA_EC_CUSTOM_CHANNEL_ID, id_ec_mode_1_actuation_sensitivity,
                               (void *)EC_CONFIG_MODE_1_ACTUATION_SENSITIVITY,
                               eeprom_ec_config.mode_1_actuation_sensitivity);
      break;
    case id_ec_mode_1_release_sensitivity:
      // range
      ec_config.mode_1_release_sensitivity = value_data[0];
      eeprom_ec_config.mode_1_release_sensitivity = ec_config.mode_1_release_sensitivity;
      defer_eeprom_update_byte(VIA_EC_CUSTOM_CHANNEL_ID, id_ec_mode_1_release_sensitivity,
                               (void *)EC_CONFIG_MODE_1_RELEASE_SENSITIVITY,
                               eeprom_ec_config.mode_1_release_sensitivity);
      break;
    case id_ec_bottoming_calibration:
      ec_config.bottoming_calibration = value_data[0];
      if (ec_config.bottoming_calibration) {
        // start calibration
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
          for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            ec_config.bottoming_calibration_starter[row][col] = true;
          }
        }
      } else {
        // end calibration
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
          for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (!ec_config.bottoming_calibration_starter[row][col]) {
              eeprom_ec_config.bottoming_reading[row][col] = ec_config.bottoming_reading[row][col];
            };
          }
        }
        ec_config_rescale();
        eeprom_update_block(&eeprom_ec_config.bottoming_reading[0][0], (void *)EC_CONFIG_BOTTOMING_READING,
                            MATRIX_COLS * MATRIX_ROWS * 2);
      }
      break;
      // toggle
    case id_ec_show_calibration_data:
      if (value_data[0] != 0) {
        defer_show_calibration_data(3000UL);
      }
      break;
  }
}

// Handle the data sent by the keyboard to the VIA menus
static void ec_config_get_value(uint8_t *data) {
  // data = [ value_id, value_data ]
  uint8_t *value_id = &(data[0]);
  uint8_t *value_data = &(data[1]);

  switch (*value_id) {
    case id_ec_actuation_mode:
      // dropdown
      value_data[0] = ec_config.actuation_mode;
      break;
    case id_ec_mode_0_actuation_threshold:
      // range
      value_data[0] = ec_config.mode_0_actuation_threshold >> 8;
      value_data[1] = ec_config.mode_0_actuation_threshold & 0xff;
      break;
    case id_ec_mode_0_release_threshold:
      // range
      value_data[0] = ec_config.mode_0_release_threshold >> 8;
      value_data[1] = ec_config.mode_0_release_threshold & 0xff;
      break;
    case id_ec_mode_1_initial_deadzone_offset:
      // range
      value_data[0] = ec_config.mode_1_initial_deadzone_offset >> 8;
      value_data[1] = ec_config.mode_1_initial_deadzone_offset & 0xff;
      break;
    case id_ec_mode_1_actuation_sensitivity:
      // range
      value_data[0] = ec_config.mode_1_actuation_sensitivity;
      break;
    case id_ec_mode_1_release_sensitivity:
      // range
      value_data[0] = ec_config.mode_1_release_sensitivity;
      break;
    case id_ec_bottoming_calibration:
      // toggle
      value_data[0] = ec_config.bottoming_calibration;
      break;
    case id_ec_show_calibration_data:
      // toggle
      value_data[0] = 0;
      break;
  }
}

static void _send_matrix_array(void *matrix_array, uint8_t size, bool is_bool, bool is_c) {
  send_string(is_c ? "{\n" : "[\n");
  for (int row = 0; row < MATRIX_ROWS; row++) {
    send_string(is_c ? "{" : "[");
    for (int col = 0; col < MATRIX_COLS; col++) {
      if (is_bool) {
        send_string(*(bool *)matrix_array ? "true" : "false");
      } else {
        send_string("0x");
        if (size == 1) {
          send_byte(*(uint8_t *)matrix_array);
        } else if (size == 2) {
          send_word(*(uint16_t *)matrix_array);
        }
      }
      if (col < (MATRIX_COLS - 1)) {
        send_string(",");
      }
      matrix_array += size;
      wait_ms(20);
    }
    send_string(is_c ? "}" : "]");
    if (row < (MATRIX_ROWS - 1)) {
      send_string(",");
    }
    send_string("\n");
  }
  send_string(is_c ? "}" : "]");
  wait_ms(40);
}

static void ec_send_config(void) {
  send_string("const data = {\nec_config_initialized: ");
  send_string(ec_config_initialized ? "true" : "false");
  send_string(",\nec_config_error: ");
  send_word(ec_config_error);
  send_string(",\nsw_value: ");
  _send_matrix_array(&sw_value[0][0], 2, false, false);
  send_string(",\nec_config: {\nactuation_mode: 0x");
  send_byte(ec_config.actuation_mode);
  send_string(",\nmode_0_actuation_threshold: 0x");
  send_word(ec_config.mode_0_actuation_threshold);
  send_string(",\nmode_0_release_threshold: 0x");
  send_word(ec_config.mode_0_release_threshold);
  send_string(",\nmode_1_initial_deadzone_offset: 0x");
  send_word(ec_config.mode_1_initial_deadzone_offset);

  send_string(",\nrescaled_mode_0_actuation_threshold: ");
  _send_matrix_array(&ec_config.rescaled_mode_0_actuation_threshold[0][0], 2, false, false);
  send_string(",\nrescaled_mode_0_release_threshold: ");
  _send_matrix_array(&ec_config.rescaled_mode_0_release_threshold[0][0], 2, false, false);

  send_string(",\nrescaled_mode_1_initial_deadzone_offset: ");
  _send_matrix_array(&ec_config.rescaled_mode_1_initial_deadzone_offset[0][0], 2, false, false);
  send_string(",\nmode_1_actuation_sensitivity: 0x");
  send_byte(ec_config.mode_1_actuation_sensitivity);
  send_string(",\nmode_1_release_sensitivity: 0x");
  send_byte(ec_config.mode_1_release_sensitivity);
  send_string(",\nextremum: ");
  _send_matrix_array(&ec_config.extremum[0][0], 2, false, false);
  send_string(",\nnoise_floor: ");
  _send_matrix_array(&ec_config.noise_floor[0][0], 2, false, false);
  send_string(",\nbottoming_calibration: ");
  send_string(ec_config.bottoming_calibration ? "true" : "false");
  send_string(",\nbottoming_calibration_starter: ");
  _send_matrix_array(&ec_config.bottoming_calibration_starter[0][0], sizeof(bool), true, false);
  send_string(",\nbottoming_reading: ");
  _send_matrix_array(&ec_config.bottoming_reading[0][0], 2, false, false);

  send_string("\n}\n}\n/*\nconst uint16_t PROGMEM caliblated_bottming_reading[MATRIX_ROWS][MATRIX_COLS] = ");
  _send_matrix_array(&ec_config.bottoming_reading[0][0], 2, false, true);
  send_string(";\n*/\n");
}

static uint32_t defer_show_calibration_data_cb(uint32_t trigger_time, void *cb_arg) {
  ec_send_config();
  show_calibration_data_token = 0;
  return 0;
}

static void defer_show_calibration_data(uint32_t delay_ms) {
  show_calibration_data_token = defer_exec(delay_ms, &defer_show_calibration_data_cb, NULL);
}
