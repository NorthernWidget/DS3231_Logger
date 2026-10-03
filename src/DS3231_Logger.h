/******************************************************************************
DS3231_Logger.h
A simplified library for the DS3231, focused on data logger applications
Bobby Schulz @ Northern Widget LLC
4/4/2018

The DS3231 is a high accuracy tempurature compensated RTC. This chip allows for time to be accurately kept
over long periods of time, and waking up a logger device when required to take measurments or tend to sensors

"That's not fair. That's not fair at all. There was time now. There was, was all the time I needed..."
-Henery Bemis

Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef DS3231_LOGGER_h
#define DS3231_LOGGER_h

#include "Arduino.h"
#ifdef __AVR__
#include <avr/pgmspace.h>  //The format strings live in flash, not in RAM
#else
#define snprintf_P snprintf  //A host build, where a literal is in RAM anyway
#endif
#include <Wire.h>

#define SECOND 5
#define MINUTE 4
#define HOUR 3
#define DAY 2
#define MONTH 1
#define YEAR 0

class DS3231_Logger {
public:
  DS3231_Logger();
  int begin(void);
  int setTime(int Year, int Month, int Day, int Hour, int Min, int Sec);

  /**
		 * @brief Read the clock's registers into the stored fields.
		 * @details What getValue() and setAlarm() need, and what formatTime()
		 * formats. Separated from the formatting so that nothing has to build a
		 * string to find out what time it is.
		 */
  void readTime();

  /**
		 * @brief Write the stored time into a buffer, in one of four formats.
		 * @details Reads nothing: call readTime() first. This is the one
		 * definition of each format; printTime() and getTime() both come
		 * through it.
		 * @param buf Where to write; truncated rather than overrun.
		 * @param n Size of buf, including the terminator. 32 is always enough.
		 * @param mode 0 scientific (YYYY/MM/DD HH:MM:SS), 1 US civilian,
		 *        2 US civilian with a 12-hour clock, 1701 stardate.
		 * @return Characters written, not counting the terminator.
		 */
  size_t formatTime(char* buf, size_t n, int mode = 0);

  /**
		 * @brief Read the clock and print the time into any Print.
		 * @details A File to reach a card, Serial to reach the monitor. No string
		 * is built for it.
		 * @return Bytes printed.
		 */
  size_t printTime(Print& out, int mode = 0);

  /** @brief Read the clock and return the time as a String, in formatTime()'s formats. */
  String getTime(int mode);
  float getTemp();
  int getValue(int n);
  int setAlarm(unsigned int Seconds);
  int clearAlarm();

private:
  int ADR = 0x68;    //Address of DS3231 (non-variable)
  int Time_Date[6];  //Store date time values of integers
};

#endif
