/* Copyright 2021 @ lokher (https://www.keychron.com)
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
#include "indicator.h"

#include <eeprom.h>

#include "bat_level_animation.h"
#include "battery.h"
#include "bluetooth_config.h"
#include "eeprom.h"
#include "i2c_master.h"
#include "rgb_matrix.h"
#include "rtc_timer.h"
#include "transport.h"

#define DECIDE_TIME(t, duration) (duration == 0 ? RGB_MATRIX_TIMEOUT_INFINITE : ((t > duration) ? t : duration))

#define LED_ON 0x80
#define INDICATOR_SET(s) memcpy(&indicator_config, &s##_config, sizeof(indicator_config_t));

enum {
  BACKLIGHT_OFF = 0x00,
  BACKLIGHT_ON_CONNECTED = 0x01,
  BACKLIGHT_ON_UNCONNECTED = 0x02,
};

static indicator_config_t pairing_config = INDICATOR_CONFIG_PARING;
static indicator_config_t connected_config = INDICATOR_CONFIG_CONNECTD;
static indicator_config_t reconnecting_config = INDICATOR_CONFIG_RECONNECTING;
static indicator_config_t disconnected_config = INDICATOR_CONFIG_DISCONNECTED;
indicator_config_t indicator_config;
static bluetooth_state_t indicator_state;
static uint16_t next_period;
static indicator_type_t type;
static uint32_t indicator_timer_buffer = 0;

#if defined(BAT_LOW_LED_PIN)
static uint32_t bat_low_pin_indicator = 0;
static uint32_t bat_low_blink_duration = 0;
#endif

#if defined(LOW_BAT_IND_INDEX)
static uint32_t bat_low_backlit_indicator = 0;
static uint8_t bat_low_ind_state = 0;
static uint32_t rtc_time = 0;
#endif

backlight_state_t original_backlight_state;

static uint8_t host_led_matrix_list[HOST_DEVICES_COUNT] = HOST_LED_MATRIX_LIST;

#ifdef HOST_LED_PIN_LIST
static pin_t host_led_pin_list[HOST_DEVICES_COUNT] = HOST_LED_PIN_LIST;
#endif

#define LED_DRIVER rgb_matrix_driver
#define SET_ALL_LED_OFF() rgb_matrix_set_color_all(0, 0, 0)
#define SET_LED_OFF(idx) rgb_matrix_set_color(idx, 0, 0, 0)
#define SET_LED_ON(idx) rgb_matrix_set_color(idx, 255, 255, 255)
#define SET_LED_BT(idx) rgb_matrix_set_color(idx, 0, 0, 255)
#define SET_LED_LOW_BAT(idx) rgb_matrix_set_color(idx, 255, 0, 0)
#define LED_DRIVER_EECONFIG_RELOAD()                                                     \
  eeprom_read_block(&rgb_matrix_config, EECONFIG_RGB_MATRIX, sizeof(rgb_matrix_config)); \
  if (!rgb_matrix_config.mode) {                                                         \
    eeconfig_update_rgb_matrix_default();                                                \
  }

void indicator_init(void) {
  memset(&indicator_config, 0, sizeof(indicator_config));

#ifdef HOST_LED_PIN_LIST
  for (uint8_t i = 0; i < HOST_DEVICES_COUNT; i++) {
    setPinOutput(host_led_pin_list[i]);
    writePin(host_led_pin_list[i], !HOST_LED_PIN_ON_STATE);
  }
#endif

#ifdef BAT_LOW_LED_PIN
  setPinOutput(BAT_LOW_LED_PIN);
  writePin(BAT_LOW_LED_PIN, !BAT_LOW_LED_PIN_ON_STATE);
#endif
}

void indicator_enable(void) {
  if (!rgb_matrix_is_enabled()) {
    rgb_matrix_enable_noeeprom();
  }
}

inline void indicator_disable(void) { rgb_matrix_disable_noeeprom(); }

void indicator_set_backlit_timeout(uint32_t time) { rgb_matrix_disable_timeout_set(time); }

static inline void indicator_reset_backlit_time(void) { rgb_matrix_disable_time_reset(); }

bool indicator_is_enabled(void) { return rgb_matrix_is_enabled(); }

void indicator_eeconfig_reload(void) { LED_DRIVER_EECONFIG_RELOAD(); }

bool indicator_is_running(void) {
  return
#if defined(BAT_LOW_LED_PIN)
    bat_low_blink_duration ||
#endif
#if defined(LOW_BAT_IND_INDEX)
    bat_low_ind_state ||
#endif
    !!indicator_config.value;
}

static void indicator_timer_cb(void *arg) {
  if (*(indicator_type_t *)arg != INDICATOR_LAST) type = *(indicator_type_t *)arg;

  bool time_up = false;
  switch (type) {
    case INDICATOR_NONE:
      break;
    case INDICATOR_OFF:
      next_period = 0;
      time_up = true;
      break;

    case INDICATOR_ON:
      if (indicator_config.value) {
        if (indicator_config.elapsed == 0) {
          indicator_config.value |= LED_ON;

          if (indicator_config.duration) {
            indicator_config.elapsed += indicator_config.duration;
          }
        } else
          time_up = true;
      }
      break;

    case INDICATOR_ON_OFF:
      if (indicator_config.value) {
        if (indicator_config.elapsed == 0) {
          indicator_config.value |= LED_ON;
          next_period = indicator_config.on_time;
        } else {
          indicator_config.value = indicator_config.value & 0x0F;
          next_period = indicator_config.duration - indicator_config.on_time;
        }

        if ((indicator_config.duration == 0 || indicator_config.elapsed <= indicator_config.duration) &&
            next_period != 0) {
          indicator_config.elapsed += next_period;
        } else {
          time_up = true;
        }
      }
      break;

    case INDICATOR_BLINK:
      if (indicator_config.value) {
        if (indicator_config.value & LED_ON) {
          indicator_config.value = indicator_config.value & 0x0F;
          next_period = indicator_config.off_time;
        } else {
          indicator_config.value |= LED_ON;
          next_period = indicator_config.on_time;
        }

        if ((indicator_config.duration == 0 || indicator_config.elapsed <= indicator_config.duration) &&
            next_period != 0) {
          indicator_config.elapsed += next_period;
        } else {
          time_up = true;
        }
      }
      break;
    default:
      time_up = true;

      next_period = 0;
      break;
  }

#ifdef HOST_LED_PIN_LIST
  if (indicator_config.value) {
    uint8_t idx = (indicator_config.value & 0x0F) - 1;

    if (idx < HOST_DEVICES_COUNT) {
      if ((indicator_config.value & 0x80) && !time_up) {
        writePin(host_led_pin_list[idx], HOST_LED_PIN_ON_STATE);
      } else {
        writePin(host_led_pin_list[idx], !HOST_LED_PIN_ON_STATE);
      }
    }
  }
#endif

  if (time_up) {
    /* Set indicator to off on timeup, avoid keeping light up until next update in raindrop effect */
    indicator_config.value = indicator_config.value & 0x0F;
    rgb_matrix_indicators_user();
    indicator_config.value = 0;
  }

  if (indicator_config.value == 0) {
    indicator_eeconfig_reload();
    if (!rgb_matrix_is_enabled()) indicator_disable();
  }
}

void indicator_set(bluetooth_state_t state, uint8_t host_index) {
  if (get_transport() != TRANSPORT_BLUETOOTH) return;
  dprintf("indicator set: %d, %d\n", state, host_index);

  static uint8_t current_state = 0;
  static uint8_t current_host = 0;

  bool host_index_changed = false;
  if (current_host != host_index && state != BLUETOOTH_DISCONNECTED) {
    host_index_changed = true;
    current_host = host_index;
  }

  if (current_state != state || host_index_changed) {
    current_state = state;
  } else {
    return;
  }

  indicator_timer_buffer = sync_timer_read32();

  /* Turn on backlight mode for indicator */
  indicator_enable();
  indicator_reset_backlit_time();

  switch (state) {
    case BLUETOOTH_DISCONNECTED:
#ifdef HOST_LED_PIN_LIST
      writePin(host_led_pin_list[host_index - 1], !HOST_LED_PIN_ON_STATE);
#endif
      INDICATOR_SET(disconnected);
      indicator_config.value = (indicator_config.type == INDICATOR_NONE) ? 0 : host_index;
      indicator_timer_cb((void *)&indicator_config.type);

      if (battery_is_critical_low()) {
        indicator_set_backlit_timeout(1000);
      } else {
        /* Set timer so that user has chance to turn on the backlight when is off */
        indicator_set_backlit_timeout(
          DECIDE_TIME(DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT * 1000, indicator_config.duration));
      }
      break;

    case BLUETOOTH_CONNECTED:
      if (indicator_state != BLUETOOTH_CONNECTED) {
        INDICATOR_SET(connected);
        indicator_config.value = (indicator_config.type == INDICATOR_NONE) ? 0 : host_index;
        indicator_timer_cb((void *)&indicator_config.type);
      }
      indicator_set_backlit_timeout(DECIDE_TIME(CONNECTED_BACKLIGHT_DISABLE_TIMEOUT * 1000, indicator_config.duration));
      break;

    case BLUETOOTH_PARING:
      INDICATOR_SET(pairing);
      indicator_config.value = (indicator_config.type == INDICATOR_NONE) ? 0 : LED_ON | host_index;
      indicator_timer_cb((void *)&indicator_config.type);
      indicator_set_backlit_timeout(
        DECIDE_TIME(DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT * 1000, indicator_config.duration));
      break;

    case BLUETOOTH_RECONNECTING:
      INDICATOR_SET(reconnecting);
      indicator_config.value = (indicator_config.type == INDICATOR_NONE) ? 0 : LED_ON | host_index;
      indicator_timer_cb((void *)&indicator_config.type);
      indicator_set_backlit_timeout(
        DECIDE_TIME(DISCONNECTED_BACKLIGHT_DISABLE_TIMEOUT * 1000, indicator_config.duration));
      break;

    case BLUETOOTH_SUSPEND:
      INDICATOR_SET(disconnected);
      indicator_config.value = (indicator_config.type == INDICATOR_NONE) ? 0 : host_index;
      indicator_timer_cb((void *)&indicator_config.type);
      indicator_set_backlit_timeout(100);
      break;

    default:
      break;
  }

  indicator_state = state;
}

void indicator_stop(void) {
  indicator_config.value = 0;
  indicator_eeconfig_reload();

  if (indicator_is_enabled()) {
    indicator_enable();
  } else {
    indicator_disable();
  }
}

#ifdef BAT_LOW_LED_PIN
void indicator_battery_low_enable(bool enable) {
  if (enable) {
    if (bat_low_blink_duration == 0) {
      bat_low_blink_duration = bat_low_pin_indicator = sync_timer_read32() | 1;
    } else
      bat_low_blink_duration = sync_timer_read32() | 1;
  } else
    writePin(BAT_LOW_LED_PIN, !BAT_LOW_LED_PIN_ON_STATE);
}
#endif

#if defined(LOW_BAT_IND_INDEX)
void indicator_battery_low_backlit_enable(bool enable) {
  if (enable) {
    uint32_t t = rtc_timer_read_ms();
    /* Check overflow */
    if (rtc_time > t) {
      if (bat_low_ind_state == 0)
        rtc_time = t;  // Update rtc_time if indicating is not running
      else {
        rtc_time += t;
      }
    }
    /* Indicating at first time or after the interval */
    if ((rtc_time == 0 || t - rtc_time > LOW_BAT_LED_TRIG_INTERVAL) && bat_low_ind_state == 0) {
      bat_low_backlit_indicator = enable ? (timer_read32() == 0 ? 1 : timer_read32()) : 0;
      rtc_time = rtc_timer_read_ms();
      bat_low_ind_state = 1;

      indicator_enable();
    }
  } else {
    rtc_time = 0;
    bat_low_ind_state = 0;

    indicator_eeconfig_reload();
    if (!rgb_matrix_is_enabled()) indicator_disable();
  }
}
#endif

void indicator_battery_low(void) {
#ifdef BAT_LOW_LED_PIN
  if (bat_low_pin_indicator && sync_timer_elapsed32(bat_low_pin_indicator) > (LOW_BAT_LED_BLINK_PERIOD)) {
    togglePin(BAT_LOW_LED_PIN);
    bat_low_pin_indicator = sync_timer_read32() | 1;
    // Turn off low battery indication if we reach the duration
    if (sync_timer_elapsed32(bat_low_blink_duration) > LOW_BAT_LED_BLINK_DURATION &&
        palReadLine(BAT_LOW_LED_PIN) != BAT_LOW_LED_PIN_ON_STATE) {
      bat_low_blink_duration = bat_low_pin_indicator = 0;
    }
  }
#endif
#if defined(LOW_BAT_IND_INDEX)
  if (bat_low_ind_state) {
    if ((bat_low_ind_state & 0x0F) <= (LOW_BAT_LED_BLINK_TIMES) &&
        sync_timer_elapsed32(bat_low_backlit_indicator) > (LOW_BAT_LED_BLINK_PERIOD)) {
      if (bat_low_ind_state & 0x80) {
        bat_low_ind_state &= 0x7F;
        bat_low_ind_state++;
      } else {
        bat_low_ind_state |= 0x80;
      }

      bat_low_backlit_indicator = sync_timer_read32() == 0 ? 1 : sync_timer_read32();

      /*  Restore backligth state */
      if ((bat_low_ind_state & 0x0F) > (LOW_BAT_LED_BLINK_TIMES)) {
#  if defined(NUM_LOCK_INDEX) || defined(CAPS_LOCK_INDEX) || defined(SCROLL_LOCK_INDEX) || \
    defined(COMPOSE_LOCK_INDEX) || defined(KANA_LOCK_INDEX)
        if (rgb_matrix_driver_allow_shutdown())
#  endif
          indicator_disable();
      }
    } else if ((bat_low_ind_state & 0x0F) > (LOW_BAT_LED_BLINK_TIMES)) {
      bat_low_ind_state = 0;
    }
  }
#endif
}

void indicator_task(void) {
  bat_level_animiation_task();

  if (indicator_config.value && sync_timer_elapsed32(indicator_timer_buffer) >= next_period) {
    indicator_timer_cb((void *)&type);
    indicator_timer_buffer = sync_timer_read32();
  }

  indicator_battery_low();
}

static void os_state_indicate(void) {
#if defined(NUM_LOCK_INDEX)
  if (host_keyboard_led_state().num_lock) {
    SET_LED_ON(NUM_LOCK_INDEX);
  }
#endif
#if defined(CAPS_LOCK_INDEX)
  if (host_keyboard_led_state().caps_lock) {
#  if defined(DIM_CAPS_LOCK)
    SET_LED_OFF(CAPS_LOCK_INDEX);
#  else
    SET_LED_ON(CAPS_LOCK_INDEX);
#  endif
  }
#endif
#if defined(SCROLL_LOCK_INDEX)
  if (host_keyboard_led_state().scroll_lock) {
    SET_LED_ON(SCROLL_LOCK_INDEX);
  }
#endif
#if defined(COMPOSE_LOCK_INDEX)
  if (host_keyboard_led_state().compose) {
    SET_LED_ON(COMPOSE_LOCK_INDEX);
  }
#endif
#if defined(KANA_LOCK_INDEX)
  if (host_keyboard_led_state().kana) {
    SET_LED_ON(KANA_LOCK_INDEX);
  }
#endif
}

bool rgb_matrix_indicators_user(void) {
  if (get_transport() == TRANSPORT_BLUETOOTH) {
    /* Prevent backlight flash caused by key activities */
    if (battery_is_critical_low()) {
      SET_ALL_LED_OFF();
      return false;
    }

#if (defined(LED_MATRIX_ENABLE) || defined(RGB_MATRIX_ENABLE)) && defined(LOW_BAT_IND_INDEX)
    if (battery_is_empty()) SET_ALL_LED_OFF();
    if (bat_low_ind_state && (bat_low_ind_state & 0x0F) <= LOW_BAT_LED_BLINK_TIMES) {
      if (bat_low_ind_state & 0x80)
        SET_LED_LOW_BAT(LOW_BAT_IND_INDEX);
      else
        SET_LED_OFF(LOW_BAT_IND_INDEX);
    }
#endif
    if (bat_level_animiation_actived()) {
      bat_level_animiation_indicate();
    }
    static uint8_t last_host_index = 0xFF;

    if (indicator_config.value) {
      uint8_t host_index = indicator_config.value & 0x0F;

      if (indicator_config.highlight) {
        SET_ALL_LED_OFF();
      } else if (last_host_index != host_index) {
        SET_LED_OFF(host_led_matrix_list[last_host_index - 1]);
        last_host_index = host_index;
      }

      if (indicator_config.value & 0x80) {
        SET_LED_BT(host_led_matrix_list[host_index - 1]);
      } else {
        SET_LED_OFF(host_led_matrix_list[host_index - 1]);
      }
    } else
      os_state_indicate();

  } else
    os_state_indicate();

  return false;
}

bool led_update_user(led_t led_state) {
  if (!rgb_matrix_is_enabled()) {
#ifdef RGB_MATRIX_DRIVER_SHUTDOWN_ENABLE
    LED_DRIVER.exit_shutdown();
#endif
    SET_ALL_LED_OFF();
    os_state_indicate();
    LED_DRIVER.flush();
#ifdef RGB_MATRIX_DRIVER_SHUTDOWN_ENABLE
    if (rgb_matrix_driver_allow_shutdown()) LED_DRIVER.shutdown();
#endif
  }
  return true;
}

void rgb_matrix_none_indicators_user(void) { os_state_indicate(); }

#ifdef RGB_MATRIX_DRIVER_SHUTDOWN_ENABLE
bool rgb_matrix_driver_allow_shutdown(void) {
#  if defined(NUM_LOCK_INDEX)
  if (host_keyboard_led_state().num_lock) return false;
#  endif
#  if defined(CAPS_LOCK_INDEX) && !defined(DIM_CAPS_LOCK)
  if (host_keyboard_led_state().caps_lock) return false;
#  endif
#  if defined(SCROLL_LOCK_INDEX)
  if (host_keyboard_led_state().scroll_lock) return false;
#  endif
#  if defined(COMPOSE_LOCK_INDEX)
  if (host_keyboard_led_state().compose) return false;
#  endif
#  if defined(KANA_LOCK_INDEX)
  if (host_keyboard_led_state().kana) return false;
#  endif
  return true;
}
#endif
