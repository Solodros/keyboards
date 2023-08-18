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

#pragma once

#define MATRIX_ROWS 5
#define MATRIX_COLS 15

#define MATRIX_ROW_PINS \
  { B15, A8, B0, A7, B1 }

#define AMUX_COUNT 2
#define AMUX_MAX_COLS_COUNT 8

#define AMUX_EN_PINS \
  { B7, B3 }

#define AMUX_SEL_PINS \
  { B6, B5, B4 }

#define AMUX_COL_CHANNELS_SIZES \
  { 8, 7 }

#define AMUX_0_COL_CHANNELS \
  { 0, 3, 1, 2, 5, 7, 6, 4 }

#define AMUX_1_COL_CHANNELS \
  { 0, 3, 1, 2, 5, 7, 6 }

#define AMUX_COL_CHANNELS AMUX_0_COL_CHANNELS, AMUX_1_COL_CHANNELS

#define DISCHARGE_PIN A6
#define ANALOG_PORT A3

#define DEFAULT_ACTUATION_MODE 0
#define DEFAULT_MODE_0_ACTUATION_LEVEL 550
#define DEFAULT_MODE_0_RELEASE_LEVEL 500
#define DEFAULT_MODE_1_INITIAL_DEADZONE_OFFSET DEFAULT_MODE_0_ACTUATION_LEVEL
#define DEFAULT_MODE_1_ACTUATION_SENSITIVITY 70
#define DEFAULT_MODE_1_RELEASE_SENSITIVITY 70
#define DEFAULT_EXTREMUM 1023
#define EXPECTED_NOISE_FLOOR 0
#define DEFAULT_NOISE_FLOOR_SAMPLING_COUNT 30
#define DEFAULT_BOTTOMING_READING 1023
#define DEFAULT_CALIBRATION_STARTER true

#define DISCHARGE_TIME 10

/* use calibrated bottming value as default */
#define ENABLE_CALIBRATED_BOTTOMING_READING

// #define VIA_EC_CUSTOM_CHANNEL_ID id_custom_channel_user_range
#define VIA_EC_CUSTOM_CHANNEL_ID 16

/* increase eeprom size */
#define WEAR_LEVELING_LOGICAL_SIZE 8192
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)

/* I use KB scope for common library */
// #define EECONFIG_KB_DATA_SIZE 160
#define EECONFIG_USER_DATA_SIZE (10 + MATRIX_COLS * MATRIX_ROWS * 2)

/* ViA layout options */
/*  7 bit */
#define LAYOUT_OPTION_SPLIT_BS (1 << 7)

/*  6 bit */
#define LAYOUT_OPTION_SPLIT_LSHIFT (1 << 6)

/*  4-5 bit */
#define LAYOUT_OPTION_SPLIT_RSHIFT_UNIFIED (0 << 4)
#define LAYOUT_OPTION_SPLIT_RSHIFT_1U_1_75U (1 << 4)
#define LAYOUT_OPTION_SPLIT_RSHIFT_1_75U_1U (2 << 4)

/*  3 bit */
#define LAYOUT_OPTION_ENTER_JIS_ISO (0 << 3)
#define LAYOUT_OPTION_ENTER_ANSI (1 << 3)

/*  0-2 bit */
#define LAYOUT_OPTION_BOTTOM_ROW_JIS 0
#define LAYOUT_OPTION_BOTTOM_ROW_TRUE_HHKB 1
#define LAYOUT_OPTION_BOTTOM_ROW_TRUE_HHKB_3U_3U 2
#define LAYOUT_OPTION_BOTTOM_ROW_RF_6U 3
#define LAYOUT_OPTION_BOTTOM_ROW_7U 4
#define LAYOUT_OPTION_BOTTOM_ROW_7U_HHKB 5

#if __has_include("secure_config.h")
#  include "secure_config.h"
#else
#  define VIA_FIRMWARE_VERSION 0
#  define SECURE_UNLOCK_SEQUENCE \
    {                            \
      {0, 0}, { 2, 13 }          \
    }
#endif

#define CUSTOM_CONFIG_RHID_DEFAULT true
