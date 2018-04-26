/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/
#include <arduino.h>
#include <RTClib.h>          // The RTC_DS1307 battery backuped RTC
#include <Wire.h>      // I2C
#include "global.h"
#include "TimeUtils.h"
#include <PinChangeInt.h>    // RTC generates 1Hz on a pin

using namespace timeUtilsNamespace;

RTC_DS1307 g_rtc;
bool TimeUtils::_timeUpdate;

void sqwInterrupt();

/*************************************************
* init
*
* This method initializes the RTC causing 1Hz interrupt
*
*
* @param      none
* @return     none
*
*************************************************/
void TimeUtils::init(void)
{
   Wire.begin();                                        // Initialize I2C
   g_rtc.writeSqwPinMode(SquareWave1HZ);                // enable 1Hz square wave
   attachInterrupt(0, sqwInterrupt, FALLING);
   _timeUpdate = false;
}

/*************************************************
* adjustForDst
*
* This method adjust the time for DST if necessary
* Most of the United States begins Daylight Saving Time at 
* 2:00 a.m. on the second Sunday in March and reverts to 
* standard time on the first Sunday in November. In the 
* U.S., each time zone switches at a different time.
*
* @param      none
* @return     none
*
*************************************************/
void TimeUtils::adjustForDst(void)
{
  
}
 
/*************************************************
* setRtc
*
* This method sets the RTC from an epoch UTC value (modified by timezone)
*
*
* @param      none
* @return     none
*
*************************************************/
void TimeUtils::setRtc(time_t timeUtc)
{
   time_t timeTz = timeUtc + (OneHour_secs * TimezoneOffset);
   g_rtc.adjust(DateTime(year(timeTz), 
                         month(timeTz), 
                         day(timeTz), 
                         hour(timeTz), 
                         minute(timeTz), 
                         second(timeTz)));   

   g_rtc.writeSqwPinMode(SquareWave1HZ);    // need to re-enable 1Hz square wave 
   DebugPrint("Setting RTC Month: ");
   DebugPrintln(month(timeTz));
}

/*************************************************
* setRtc
*
* This method sets the RTC from an epoch UTC value (modified by timezone)
*
*
* @param      none
* @return     none
*
*************************************************/
void TimeUtils::getTime(uint8_t *_hours, uint8_t *_minutes, uint8_t *_seconds, uint8_t *_dow, uint8_t *_day, uint8_t *_month)
{
   DateTime now = g_rtc.now();
   
   *_hours   = now.hour();
   *_minutes = now.minute();
   *_seconds = now.second();
   *_dow     = now.rtc_dayOfWeek();
   *_day     = now.day();
   *_month   = now.month();
}

/*************************************************
* sqwInterrupt
*
* This method handles the interrupt from the 1 Hz RTC
*
*
* @param      none
* @return     none
*
*************************************************/
void sqwInterrupt()
{
  TimeUtils::_timeUpdate = true;
}
