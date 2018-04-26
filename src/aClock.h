/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#ifndef _aClock_h
#define _aClock_h

#include <arduino.h>
#include <avr/pgmspace.h>

namespace aClockNamespace
{
// LCD
const uint8_t LcdResetPin      = 4;              // LCD uses D0, D1, and D4
const uint8_t IconHourTens     = 1;
const uint8_t IconHourOnes     = 2;
const uint8_t IconMinuteTens   = 3;
const uint8_t IconMinuteOnes   = 4;
const uint8_t IconWifi         = 5;
const uint8_t ButtonChimes     = 0;

// LCD strings
const uint8_t DebugString        = 0;
const uint8_t MonthAndDayString  = 6;
const uint8_t DayOfWeekString    = 7;
const uint8_t TempfString        = 8;
const uint8_t TempfDirection     = 9;

const char DayString[7][10]  = {" Sunday", 
                               " Monday", 
                               " Tuesday", 
                               "Wednesday", 
                               " Thursday", 
                               " Friday", 
                               " Saturday"};

const char MonthString[13][10]  =  {"", 
                                      " January", 
                                      " February", 
                                       "  March", 
                                       "  April", 
                                       "  May", 
                                       "  June", 
                                       "  July", 
                                       " August", 
                                       "September", 
                                       "October", 
                                       "November", 
                                       "December"};

const uint8_t  SoundConnect               = 0;
const uint8_t  SoundHourChime             = 1;
const uint8_t  SoundHourPreChime          = 2;
const uint16_t SoundHourPreChimeLenght_ms = 16000;
const uint16_t SoundHourChimeLenght_ms    = 4000;

// LED
const unsigned short LedPin = A0;

// Program
const uint8_t  MaxHours             = 12;
const uint8_t  MaxMinutes           = 59;
const uint8_t  MaxSeconds           = 50;
const uint32_t TicksInSecond        = 1034482;
const bool     DstActive            = false;    // @todo: Automatically set this some how
const uint16_t SecondsToUpdateTemp  = 292;      // the scope display is 200 wide, try and update the entire display in 24 hours.
const uint8_t  OneMinute_secs       = 60;
const uint8_t  InvalidDow           = 255;
const uint16_t OneSecond            = 1000;     // ms in a second

// RTC
const uint8_t  CsRtc                = 8;        // chip select 
const uint8_t  SqwIntPin            = 2;        // Pin D2 is 1Hz from RTC

const uint8_t tempDirectionIndexRising  = 0;
const uint8_t tempDirectionIndexFalling = 1;
const uint8_t tempDirectionIndexSteady  = 2;
const char tempDirectionString[3][8]    = {"Rising", "Falling", "Steady"};
}

class Aclock
{
   public:
      Aclock(){};
      ~Aclock(){};
      void init();
      void initLed(void);
      void initLcd(void);
      void initRtc(void);
      void updateTimeDisplay(void);
      void updateMonthAndDayDisplay(void);
      void updateDayOfWeekDisplay(void);
      void getAndUpdateTempDisplay(void);
      void displayTempearture(double);
      void getTemperature(void);
      void updateChime(void);
      void blinkLed(void);
      bool connectToWifi(void);
      void setRtcWithNtp(void);
      void mainLoop();

      uint8_t    _hours;
      uint8_t    _minutes;
      uint8_t    _seconds;
      uint8_t    _dow;
      uint8_t    _day;
      uint8_t    _month;
      uint8_t    _displayedDow;  // will update if it is invalid
      bool       _chimeEnable;

   private:
      uint8_t    _displayedDay;  // used to only update disoplay on change
      void       everySecond(void);
      void       everyMinute(void);
      void       everyFifteenMinutes(void);
      void       everyDay(void);

};
#endif
