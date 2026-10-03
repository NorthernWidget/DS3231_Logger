/******************************************************************************
DS3231_Logger.cpp
A simplified library for the DS3231, focused on data logger applications
Bobby Schulz @ Northern Widget LLC
4/4/2018

The DS3231 is a high accuracy tempurature compensated RTC. This chip allows for time to be accurately kept
over long periods of time, and waking up a logger device when required to take measurments or tend to sensors

"That's not fair. That's not fair at all. There was time now. There was, was all the time I needed..."
-Henery Bemis

Distributed as-is; no warranty is given.
******************************************************************************/

#include "Arduino.h"
#include <Wire.h>
#include <DS3231_Logger.h>

DS3231_Logger::DS3231_Logger()
{

}

int DS3231_Logger::begin(void)
{
	Wire.begin();

	Wire.beginTransmission(ADR);
	Wire.write(0x0E); //Write values to Control reg
	Wire.write(0x24); //Start oscilator, turn off BBSQW, Turn off alarms, turn on convert
	return Wire.endTransmission(); //return result of begin, reading is optional
}

int DS3231_Logger::setTime(int Year, int Month, int Day, int Hour, int Min, int Sec)
{
	if(Year > 999) {
		Year = Year - 2000; //FIX! Add compnesation for centry 
	}
	int TimeDate [7]={Sec,Min,Hour,0,Day,Month,Year};
	for(int i=0; i<=6;i++){
		if(i==3)
			i++;
		int b= TimeDate[i]/10;
		int a= TimeDate[i]-b*10;
		if(i==2){
			if (b==2)
				b=B00000010;
			else if (b==1)
				b=B00000001;
		}	
		TimeDate[i]= a+(b<<4);
		  
		Wire.beginTransmission(ADR);
		Wire.write(i); //Write values starting at reg 0x00
		Wire.write(TimeDate[i]); //Write time date values into regs
		Wire.endTransmission(); //return result of begin, reading is optional
  }

  //Read back time to test result of write??
}

void DS3231_Logger::readTime()
{
	int TimeDate [7]; //second,minute,hour,null,day,month,year
	Wire.beginTransmission(ADR); //Ask 1 byte of data
	Wire.write(0x00); //Read values starting at reg 0x00
	Wire.endTransmission();
	Wire.requestFrom(ADR, 7);
	for(int i=0; i<=6;i++){
		if(i==3) {
			i++;
			Wire.read();
		}

		unsigned int n = Wire.read(); //Read value of reg

		//Process results
		int a=n & B00001111;
		if(i==2){
			int b=(n & B00110000)>>4; //24 hour mode
			if(b==B00000010)
				b=20;
			else if(b==B00000001)
				b=10;
			TimeDate[i]=a+b;
		}
		else if(i==4){
			int b=(n & B00110000)>>4;
			TimeDate[i]=a+b*10;
		}
		else if(i==5){
			int b=(n & B00010000)>>4;
			TimeDate[i]=a+b*10;
		}
		else if(i==6){
			int b=(n & B11110000)>>4;
			TimeDate[i]=a+b*10;
		}
		else{
			int b=(n & B01110000)>>4;
			TimeDate[i]=a+b*10;
			}
	}

	Time_Date[0] = TimeDate[6];
	Time_Date[1] = TimeDate[5];
	Time_Date[2] = TimeDate[4];
	Time_Date[3] = TimeDate[2];
	Time_Date[4] = TimeDate[1];
	Time_Date[5] = TimeDate[0];
}

size_t DS3231_Logger::formatTime(char* buf, size_t n, int mode)
{
	//The stored fields, in order: 2-digit year, month, day, hour, minute, second.
	//Every field is zero-padded to two digits and the year carries its century,
	//which is what the String form did by padding each one and prefixing "20".
	int year = 2000 + Time_Date[YEAR];
	int written = 0;

	if(mode == 0) //Year, Month, Day, Hour, Minute, Second (Scientific Style)
		written = snprintf_P(buf, n, PSTR("%04d/%02d/%02d %02d:%02d:%02d"), year,
			Time_Date[MONTH], Time_Date[DAY], Time_Date[3], Time_Date[4], Time_Date[5]);

	else if(mode == 1) //Month, Day, Year, Hour, Minute, Second (US Civilian Style)
		written = snprintf_P(buf, n, PSTR("%02d/%02d/%04d %02d:%02d:%02d"),
			Time_Date[MONTH], Time_Date[DAY], year, Time_Date[3], Time_Date[4], Time_Date[5]);

	else if(mode == 2) //As mode 1, on a 12 hour clock. The hour is not padded
	{
		int hour12 = Time_Date[3] % 12;
		if(hour12 == 0) hour12 = 12;   //Midnight and noon are both twelve
		written = snprintf_P(buf, n, PSTR("%02d/%02d/%04d %d:%02d:%02d %s"),
			Time_Date[MONTH], Time_Date[DAY], year, hour12,
			Time_Date[4], Time_Date[5], Time_Date[3] >= 12 ? "PM" : "AM");
	}

	else if(mode == 1701) //Year, Day (of year), Hour, Minute, Second (Stardate)
	{
		int DayOfYear = 0;
		int MonthDay[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if(Time_Date[YEAR] % 4 == 0) MonthDay[2] = 29;

		for(int m = 1; m < Time_Date[MONTH]; m++)
		{
			DayOfYear = DayOfYear + MonthDay[m];
		}
		DayOfYear = DayOfYear + Time_Date[DAY];

		written = snprintf_P(buf, n, PSTR("%04d.%d %02d.%02d.%04d"), year, DayOfYear,
			Time_Date[DAY], Time_Date[MONTH], year);
	}

	else written = snprintf_P(buf, n, PSTR("Invalid Input"));

	if(written < 0) { //The platform refused to format at all
		if(n > 0) buf[0] = '\0';
		return 0;
	}
	if((size_t)written >= n) return n > 0 ? n - 1 : 0; //Truncated: say what fits
	return (size_t)written;
}

size_t DS3231_Logger::printTime(Print& out, int mode)
{
	readTime();
	char buf[32];
	formatTime(buf, sizeof(buf), mode);
	return out.print(buf);
}

String DS3231_Logger::getTime(int mode)
{
	readTime();
	char buf[32];
	formatTime(buf, sizeof(buf), mode);
	return String(buf);
}


float DS3231_Logger::getTemp()
{
	float Temp = 0;
	Wire.beginTransmission(ADR);
	Wire.write(0x11); //Read from reg 0x11
	Wire.endTransmission();

	Wire.requestFrom(ADR, 2);
	uint8_t TempHigh = Wire.read(); //Get high reg of temp data
	uint8_t TempLow = Wire.read();	//Get low reg of temp data

	if(bitRead(TempHigh, 7) == 1) {
	TempHigh = (~TempHigh) + 1;  //Take 2s complement of whole temp value
	Temp = -1.0*float(TempHigh + 0.25*float(TempLow >> 6)); //Temp = -(Whole + 2^-2 x Frac)
	}
	else Temp = float(TempHigh) + 0.25*float(TempLow >> 6);	//Temp = (Whole + 2^-2 x Frac)

	return Temp; 
}

int DS3231_Logger::getValue(int n)	// n = 0:Year, 1:Month, 2:Day, 3:Hour, 4:Minute, 5:Second
{
	readTime(); //Update time
	return Time_Date[n]; //Return desired value 
}

int DS3231_Logger::setAlarm(unsigned int Seconds) { //Set alarm from current time to x seconds from current time 
	//DEFINE LIMITS FOR FUNCTION!!

	if(Seconds == 60) {
		uint8_t AlarmMask = 0x07; //nibble for A1Mx values

		// Wire.beginTransmission(ADR);
		// Wire.write(0x0E); //Write values to control reg
		// Wire.write(0x40); //Turn on 1 Hz square wave
		// Wire.endTransmission(); 

		Wire.beginTransmission(ADR);
		Wire.write(0x0E); //Write values to control reg
		Wire.write(0x06); //Turn on INTCN and Alarm 2
		Wire.endTransmission(); 

		//DEBUG!
		Wire.beginTransmission(ADR);
		Wire.write(0x0F); //Write values to control reg
		Wire.write(0x00); //Clear any alarm flags, set oscilator to run
		Wire.endTransmission(); 

		for(int i=0; i < 3;i++){
			Wire.beginTransmission(ADR);
			Wire.write(0x0B + i); //Write values starting at reg 0x0B
			// Wire.write(((AlarmMask & (1 << i)) << 8)); //Write time date values into regs
			Wire.write(0x80); 
			Wire.endTransmission(); //return result of begin, reading is optional
		}
	}

	else {
	//Currently can not set timer for more than 24 hours
	uint8_t AlarmMask = 0x08; //nibble for A1Mx values
	uint8_t DY = 0; //DY/DT value 
	readTime();

	int AlarmTime[7] = {Time_Date[5], Time_Date[4], Time_Date[3], 0, Time_Date[2], Time_Date[1], Time_Date[0]};
	int AlarmVal[7] = {Seconds % 60, ((Seconds - (Seconds % 60))/60) % 60, ((Seconds - (Seconds % 3600))/3600) % 24, 0, ((Seconds - (Seconds % 86400))/86400), 0, 0};  //Remove unused elements?? FIX!
	int CarryIn = 0; //Carry value
	int CarryOut = 0; 
	int MonthDay[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  //Use??
	if(AlarmTime[6] % 400 == 0) MonthDay[2] = 29; //If year is divisable by 400, is leap year, correct days in February
	else if((AlarmTime[6] % 4 == 0) && (AlarmTime[6] % 100 != 0)) MonthDay[2] = 29; //Otherwise, if IS dividsable by 4, but NOT multiple of 100, is leap year, correct days in February

	Wire.beginTransmission(ADR);
	Wire.write(0x0E); //Write values to control reg
	Wire.write(0x05); //Turn on INTCN and Alarm 1
	Wire.endTransmission(); 

	//Calc seconds
	if(AlarmTime[0] + AlarmVal[0] >= 60) CarryOut = 1;
	AlarmTime[0] = (AlarmTime[0] + AlarmVal[0]) % 60;
	CarryIn = CarryOut; //Copy over prevous carry

	//Calc minutes
	if(AlarmTime[1] + AlarmVal[1] + CarryIn >= 60) CarryOut = 1;
	else CarryOut = 0;
	AlarmTime[1] = (AlarmTime[1] + AlarmVal[1] + CarryIn) % 60;
	CarryIn = CarryOut; //Copy over prevous carry

	//Calc hours
	if(AlarmTime[2] + AlarmVal[2] + CarryIn >= 24) CarryOut = 1; //OUT OF RANGE??
	else CarryOut = 0;
	AlarmTime[2] = (AlarmTime[2] + AlarmVal[2] + CarryIn) % 24;
	CarryIn = CarryOut; //Copy over prevous carry

	//Calc days 
	if(AlarmTime[4] + AlarmVal[4] + CarryIn > MonthDay[AlarmTime[5]]) CarryOut = 1;  //Carry out if result pushes you beyond current month 
	else CarryOut = 0;
	AlarmTime[4] = (AlarmTime[4] + AlarmVal[4] + CarryIn) % (MonthDay[AlarmTime[5]] + 1);
	if(AlarmTime[4] == 0) AlarmTime[4] = 1; //FIX! Find more elegant way to do this


	//ADD FAILURE NOTIFICATION FOR OUT OF RANGE??

	int Offset = 0;
	for(int i=0; i<=6;i++){
		if(i==3) i++;
		int b= AlarmTime[i]/10;
		int a= AlarmTime[i]-b*10;
		if(i==2){
			if (b==2)
				b=B00000010;
			else if (b==1)
				b=B00000001;
		}	
		AlarmTime[i]= a+(b<<4);
		  
		Wire.beginTransmission(ADR);
		Wire.write(0x07 + Offset); //Write values starting at reg 0x07
		Wire.write(AlarmTime[i] | ((AlarmMask & (1 << Offset)) << 8)); //Write time date values into regs
		Wire.endTransmission(); //return result of begin, reading is optional
		Offset++;
	}
	}

  clearAlarm();
}

int DS3231_Logger::clearAlarm() {  //Clear registers to stop alarm, must call setAlarm again to get it to turn on again
	Wire.beginTransmission(ADR);
	Wire.write(0x0F); //Write values to status reg
	Wire.write(0x00); //Clear all flags
	Wire.endTransmission(); //return result of begin, reading is optional
}
