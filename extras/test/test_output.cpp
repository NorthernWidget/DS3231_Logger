// Output-regression test for DS3231_Logger: compiles src/DS3231_Logger.cpp
// against the NW_Core stubs and prints every format of formatTime() for fixed
// register images. run.sh diffs the result against baseline.txt.
//
// The clock's registers are BCD (DS3231 data sheet, Table 2): seconds at 0x00,
// minutes 0x01, hours 0x02, day of week 0x03, date 0x04, month 0x05, year 0x06.
// Only mode 0 reaches a logger's files, and only mode 0 is covered by the
// NW-Sim transcripts, which is why the other three are here.
#include "Arduino.h"
#include "Wire.h"
TwoWire Wire;
#include "../../src/DS3231_Logger.cpp"

static uint8_t bcd(int v) {
  return (uint8_t)(((v / 10) << 4) | (v % 10));
}

// Put a time on the bus. `hour` is 24-hour, as the library's own decode expects
// of a clock in 24-hour mode (bit 6 of 0x02 clear).
static void clockHolds(int year2, int month, int date, int hour, int minute, int second) {
  Wire.deviceAddress = 0x68;
  Wire.image[0x00] = bcd(second);
  Wire.image[0x01] = bcd(minute);
  Wire.image[0x02] = bcd(hour);
  Wire.image[0x03] = 0x01;  // day of week: read and discarded
  Wire.image[0x04] = bcd(date);
  Wire.image[0x05] = bcd(month);
  Wire.image[0x06] = bcd(year2);
}

static void report(const char* what, DS3231_Logger& rtc) {
  char b[32];
  rtc.readTime();
  printf("[%s]\n", what);
  for (int mode : { 0, 1, 2, 1701, 7 }) {
    size_t n = rtc.formatTime(b, sizeof b, mode);
    printf("  mode %-4d %2zu bytes  %s\n", mode, n, b);
  }
  printf("  getValue: year=%d month=%d day=%d hour=%d minute=%d second=%d\n",
         rtc.getValue(YEAR), rtc.getValue(MONTH), rtc.getValue(DAY),
         rtc.getValue(3), rtc.getValue(4), rtc.getValue(5));
}

int main() {
  DS3231_Logger rtc;

  // 1. An afternoon in a common year: every field two digits, the 12-hour clock
  //    past noon, and a day of year that counts ten whole months.
  clockHolds(26, 11, 7, 14, 5, 9);
  report("2026-11-07 14:05:09", rtc);

  // 2. Midnight on the first day: the fields that need zero padding, and the
  //    hour that is twelve on a 12-hour clock rather than zero.
  clockHolds(26, 1, 1, 0, 0, 0);
  report("2026-01-01 00:00:00", rtc);

  // 3. Noon, which is the other twelve.
  clockHolds(26, 6, 30, 12, 0, 0);
  report("2026-06-30 12:00:00", rtc);

  // 4. A leap year, after February: the stardate's day of year gains a day.
  clockHolds(24, 3, 1, 23, 59, 58);
  report("2024-03-01 23:59:58", rtc);

  // 5. A buffer too small for the format: truncated, terminated, and the length
  //    reported is what fits rather than what was wanted.
  {
    clockHolds(26, 11, 7, 14, 5, 9);
    rtc.readTime();
    char small[8];
    size_t n = rtc.formatTime(small, sizeof small, 0);
    printf("[truncated to 8] %zu bytes  %s\n", n, small);
  }

  // 6. The String form, which section 15 lets this library keep: the same bytes.
  {
    rtc.readTime();
    char b[32];
    rtc.formatTime(b, sizeof b, 0);
    String s = rtc.getTime(0);
    printf("[getTime(0)] %s  matches formatTime: %d\n", s.c_str(), s == String(b));
  }

  return 0;
}
