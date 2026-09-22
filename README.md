# DS3231_Logger

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6338627.svg)](https://doi.org/10.5281/zenodo.6338627)

A simplified library for the DS3231, focused on data logger applications.

The DS3231 is a high accuracy tempurature compensated RTC. This chip allows for time to be accurately kept over long periods of time, and waking up a logger device when required to take measurments or tend to sensors.

*"That's not fair. That's not fair at all. There was time now. There was, was all the time I needed..."*
&mdash;Henery Bemis

**Installation:** included in [NorthernWidget-libraries](https://github.com/NorthernWidget/NorthernWidget-libraries).

```cpp
#include <DS3231_Logger.h>

DS3231_Logger rtc;

void setup() {
    Serial.begin(9600);
    rtc.begin();
    rtc.setTime(2026, 9, 22, 12, 0, 0);   // year, month, day, hour, minute, second (UTC)
    rtc.setAlarm(60);                     // wake the logger 60 seconds from now
}

void loop() {
    Serial.println(rtc.getTime(0));
    delay(1000);
}
```

**Full API reference:** https://docs.northernwidget.com/DS3231_Logger/
