/* Copyright 2022 Ruslan Sayfutdinov (@KapJI)
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

#include <stdint.h>

typedef enum {
  OS_UNSURE,
  OS_LINUX,
  OS_WINDOWS,
  OS_MACOS,
  OS_IOS,
} os_variant_t;

void os_detection_update_kb(os_variant_t);
void process_wlength(const uint16_t w_length);
os_variant_t detected_host_os(void);
void send_os_detection_result(void);
