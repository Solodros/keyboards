/* Copyright 2023 msasafumi
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

#include QMK_KEYBOARD_H

#ifdef EC_DEBUG
#  define _EC_DBG0 EC_DBG_DT
#  define _EC_DBG1 EC_DBG_FQ
#else
#  define _EC_DBG0 _______
#  define _EC_DBG1 _______
#endif

// tap dance
// [single tap, single hold, multi tap, tap hold, tapping term]
const tap_dance_entry_t PROGMEM tap_dance_predefined_entries[NUM_TAP_DANCE_PREDEFINED_ENTRIES] = {
  // TD(0) - Apple Fn key + IME switch
  {KC_LNG2, APPLE_FF, KC_LNG1, APPLE_FF, TAPPING_TERM},
  // TD(1) - Protect layer 3 from misstouch, MENU + MO(3)
  {KC_APP, KC_APP, KC_APP, MO(3), TAPPING_TERM},
};

// unused place holder for getting a keyrecord_t#keyccode field.
combo_t key_combos[COMBO_COUNT] = {};

// clang-format off
const uint16_t PROGMEM ec_bottoming_reading_default[MATRIX_ROWS][MATRIX_COLS] = {
  {0x0202,0x0247,0x0245,0x02ea,0x024b,0x0245,0x0207,0x0204,0x0269,0x01de,0x022a,0x0226,0x0294,0x0188,0x0209},
  {0x0241,0x028d,0x02d2,0x0317,0x0283,0x02c4,0x025e,0x0245,0x02e2,0x02a2,0x0262,0x027e,0x024c,0x01b6,0x03ff},
  {0x0195,0x02bc,0x02cc,0x0313,0x028c,0x02c3,0x0285,0x02c2,0x02e0,0x0284,0x029c,0x02ae,0x03ff,0x0244,0x03ff},
  {0x0183,0x03ff,0x0248,0x02d5,0x02dd,0x028c,0x0239,0x02cb,0x0308,0x02a5,0x02a9,0x025f,0x03ff,0x0187,0x021b},
  {0x01c1,0x01a2,0x01a1,0x03ff,0x03ff,0x03ff,0x01b7,0x03ff,0x03ff,0x03ff,0x016c,0x017d,0x01a1,0x03ff,0x03ff}
};
// clang-format on

// clang-format off
const ec_preset_t PROGMEM ec_presets_default[EC_NUM_PRESETS] = {
  // EC Preset 0 ⬜️ - default
  [0] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 1 🟩 - alphas
  [1] = {
    .actuation_mode = EC_ACTUATION_MODE_DYNAMIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel = 128,
    .release_mode = EC_RELEASE_MODE_DYNAMIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = 128,
    .deadzone = 112
  },
  // EC Preset 2 🟨 - none-alphas
  [2] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 3 🟧 - mods
  [3] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 4 🟥 - little finger home,Ctrl/HHKB fn
  [4] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 5 🟦 - enter key
  [5] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 6 🟪 - spacebar
  [6] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  },
  // EC Preset 7 ⬛️ - prevent mistouch
  [7] = {
    .actuation_mode = EC_ACTUATION_MODE_STATIC,
    .actuation_threshold = EC_ACTUATION_THRESHOLD_DEFAULT + 160,
    .actuation_travel =  EC_ACTUATION_TRAVEL_DEFAULT,
    .release_mode = EC_RELEASE_MODE_STATIC,
    .release_threshold = EC_RELEASE_THRESHOLD_DEFAULT,
    .release_travel = EC_RELEASE_TRAVEL_DEFAULT,
    .deadzone = EC_DEADZONE_DEFAULT
  }
};
// clang-format on

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // mac base layer
  [0] = LAYOUT_60_tsangan(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_ESC,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,          KC_BSPC,
    KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,                   KC_ENT,
    KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, MO(2),
    TD(0), KC_LALT, KC_LGUI,                              KC_SPC,                                               KC_RGUI, KC_RALT, TD(1)
  ),
  // standard base layer
  [1] = LAYOUT_60_tsangan(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_ESC,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,          KC_BSPC,
    KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,                   KC_ENT,
    KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, MO(2),
    TD(0),   KC_LALT, KC_LGUI,                            KC_SPC,                                               KC_RGUI, KC_RALT, TD(1)
  ),
  // HHKB-like fn layer
  [2] = LAYOUT_60_tsangan(
    _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_INS,  KC_DEL,
    KC_CAPS, KC_F16,  KC_F17,  KC_F18,  _______, _______, _______, _______, KC_PSCR, KC_SCRL, KC_PAUS, KC_UP,   _______,          _______,
    _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_EJCT, _______, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT,                   KC_PENT,
    _______,          _______, _______, _______, _______, _______, KC_PPLS, KC_PMNS, KC_END,  KC_PGDN, KC_DOWN,          _______, _______,
    _______, _______, _______,                            _______,                                              _______, _______, _______
  ),
  // settings, application-specific keys
  // row 1 - EC data
  // row 3: normal settings, +shift key for unusual reversed setting
  [3] = LAYOUT_60_tsangan(
    QK_BOOT, KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,  KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,  _______, TERM_LCK,
    _______, EC_CALD, EC_PSET, EC_PMAP, _EC_DBG0,_EC_DBG1,_______, _______, _______, _______, _______, _______, TEST_OS,          _______,
    _______, ECM(0),  ECM(1),  ECM(2),  ECM(3),  _______, _______, _______, _______, _______, _______, _______,                   _______,
    _______,          RHID_OFF,AUT_ON,  MAC_ON,  USJ_OFF, NK_ON,   CL_NORM, AG_NORM, BS_NORM, _______, _______,          _______, _______,
    _______, _______, _______,                            _______,                                              _______, _______, _______
  ),

  // EC Preset map 0
  // EC(0) default
  // EC(1) alphas
  // EC(2) none-alphas
  // EC(3) mods
  // EC(4) little finger home Ctrl/hhkb fn
  // EC(5) enter
  // EC(6) spacebar
  // EC(7) prevent mistouch
  [EC_PRESET_MAP(0)] = LAYOUT_60_tsangan(
    EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(2),
    EC(2), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1),        EC(2),
    EC(4), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1),               EC(5),
    EC(3),        EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1), EC(1),        EC(3), EC(4),
    EC(7), EC(3), EC(3),                      EC(6),                                    EC(3), EC(3), EC(7)
  ),

  // EC Preset map 1
  [EC_PRESET_MAP(1)] = LAYOUT_60_tsangan(
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),               EC(0),
    EC(0),        EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0), EC(0),
    EC(0), EC(0), EC(0),                      EC(0),                                    EC(0), EC(0), EC(0)
  ),

  // EC Preset map 2
  [EC_PRESET_MAP(2)] = LAYOUT_60_tsangan(
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),               EC(0),
    EC(0),        EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0), EC(0),
    EC(0), EC(0), EC(0),                      EC(0),                                    EC(0), EC(0), EC(0)
  ),

  // EC Preset map 3
  [EC_PRESET_MAP(3)] = LAYOUT_60_tsangan(
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0),
    EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),               EC(0),
    EC(0),        EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0), EC(0),        EC(0), EC(0),
    EC(0), EC(0), EC(0),                      EC(0),                                    EC(0), EC(0), EC(0)
  )
};
// clang-format on
