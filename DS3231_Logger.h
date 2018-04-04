#ifndef MAX_3234_h
#define MAX_3234_h

#include "Arduino.h"
#include <Wire.h>

#define SECOND 5
#define MINUTE 4
#define HOUR 3
#define DAY 2
#define MONTH 1
#define YEAR 0

class DS3231_Logger
{
	public:
		DS3231_Logger();
		int Begin(void);
		int SetTime(int Year, int Month, int Day, int Hour, int Min, int Sec);
		String GetTime(int mode);
		float GetTemp();
		int GetValue(int n);
		int SetAlarm(unsigned int Seconds);
		int ClearAlarm();

		//int GetTime();
		//String ToString();
	private:
		int ADR = 0x68; //Address of DS3231 (non-variable)
		int Time_Date[6]; //Store date time values of integers 

};

#endif