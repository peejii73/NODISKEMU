/* NODISKEMU - SD/MMC to IEEE-488 interface/controller
   Copyright (C) 2007-2018  Ingo Korb <ingo@akana.de>

   NODISKEMU is a fork of sd2iec by Ingo Korb (et al.), http://sd2iec.de

   Inspired by MMC2IEC by Lars Pontoppidan et al.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   pcf8563.c: RTC support for PCF8563 chips

   The exported functions in this file are weak-aliased to their corresponding
   versions defined in rtc.h so when this file is the only RTC implementation
   compiled in they will be automatically used by the linker.

*/

#include <stdint.h>
#include <string.h>
#include "config.h"
#include "i2c.h"
#include "progmem.h"
#include "uart.h"
#include "ustring.h"
#include "utils.h"
#include "time.h"
#include "rtc.h"
#include "pcf8563.h"

#define PCF8563_ADDR 0xa2

#define REG_CTRL1 0
#define REG_CTRL2 1
#define REG_V_SEC 2
#define REG_MIN 3
#define REG_HOUR 4
#define REG_DAY 5
#define REG_WDAY 6
#define REG_C_MONTH 7
#define REG_YEAR 8

#define REG_CLKOUT 13

#define CTRL1_STOP_CLOCK  0x20
#define CTRL1_START_CLOCK 0

/* Read the current time from the RTC */
/* Will auto-adjust the stored year if required */
void pcf8563_read(struct tm *time) {
  uint8_t  rtc_bytes[7];

  /* Set to default value in case we abort */
  memcpy_P(time, &rtc_default_date, sizeof(struct tm));
  if (rtc_state != RTC_OK)
    return;

  if (i2c_read_registers(PCF8563_ADDR, REG_V_SEC, sizeof(rtc_bytes), &rtc_bytes))
    return;

  time->tm_sec  = bcd2int(rtc_bytes[REG_V_SEC - 2] & 0x7f);
  time->tm_min  = bcd2int(rtc_bytes[REG_MIN - 2] & 0x7f);
  time->tm_hour = bcd2int(rtc_bytes[REG_HOUR - 2] & 0x3f);
  time->tm_mday = bcd2int(rtc_bytes[REG_DAY - 2] & 0x3f);
  time->tm_wday = bcd2int(rtc_bytes[REG_WDAY - 2] & 0x07);
  time->tm_mon  = bcd2int(rtc_bytes[REG_C_MONTH - 2] & 0x1f) - 1;
  time->tm_year = bcd2int(rtc_bytes[REG_YEAR - 2]);
  if (rtc_bytes[REG_C_MONTH - 2] & 0x80)
    time->tm_year += 100;
}
void read_rtc(struct tm *time) __attribute__ ((weak, alias("pcf8563_read")));

/* Set the time in the RTC */
void pcf8563_set(struct tm *time) {
  uint8_t  rtc_bytes[7];

  if (rtc_state == RTC_NOT_FOUND)
    return;

  i2c_write_register(PCF8563_ADDR, REG_CTRL1, CTRL1_STOP_CLOCK);
  rtc_bytes[REG_V_SEC - 2] = int2bcd(time->tm_sec);
  rtc_bytes[REG_MIN - 2] = int2bcd(time->tm_min);
  rtc_bytes[REG_HOUR - 2] = int2bcd(time->tm_hour);
  rtc_bytes[REG_DAY - 2] = int2bcd(time->tm_mday);
  rtc_bytes[REG_WDAY - 2] = int2bcd(time->tm_wday + 1);
  rtc_bytes[REG_C_MONTH - 2] = int2bcd(time->tm_mon);
  if (time->tm_year >= 100)
    rtc_bytes[REG_C_MONTH - 2] |= 0x80;
  rtc_bytes[REG_YEAR - 2] = int2bcd(time->tm_year % 100);

  i2c_write_registers(PCF8563_ADDR, REG_V_SEC, sizeof(rtc_bytes), &rtc_bytes);
  i2c_write_register(PCF8563_ADDR, REG_CTRL1, CTRL1_START_CLOCK);
  rtc_state = RTC_OK;
}
void set_rtc(struct tm *time) __attribute__ ((weak, alias("pcf8563_set")));

void pcf8563_init(void) {
  rtc_state = RTC_NOT_FOUND;
  uart_puts_P(PSTR("PCF8563"));
  if (i2c_write_register(PCF8563_ADDR, REG_CTRL1, CTRL1_START_CLOCK)) {
    uart_puts_P(PSTR(" not found"));
  } else {
    uint8_t sec_v;
    i2c_read_registers(PCF8563_ADDR, REG_V_SEC, 1, &sec_v);
    rtc_state = sec_v & 0x80 ? RTC_INVALID : RTC_OK;
    i2c_write_register(PCF8563_ADDR, REG_CLKOUT, 0x80); // 32768 Hz out
  }
  uart_putcrlf();
}
void rtc_init(void) __attribute__ ((weak, alias("pcf8563_init")));
