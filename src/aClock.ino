/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/
#include "global.h"
#include <arduino.h> 
#include <genieArduino.h>    // The Picaso uLCD-32PTU display
#include <SPI.h>             // The WiFi uses SPI
#include <Time.h>            // time management
#include "wifiShield.h"
#include "aClock.h"
#include "TimeUtils.h"
#include "webServer.h"
#include "freeMemory.h"
#include "weather.h"

#include <Adafruit_CC3000.h> // @todo: Not sure why this has to be here (but it does) is in wifiShield.cpp
#include <Wire.h>            // @todo: Not sure why this has to be here (but it does) is in TimeUtils.cpp
#include <RTClib.h>          // @todo: Not sure why this has to be here (but it does) is in timeUtils.cpp

using namespace aClockNamespace;
 
// classes
Genie      g_lcd;
WifiShield g_wifiShield;
Aclock     g_aClock;
TimeUtils  g_timeUtils;
WebServer  g_webServer;
Weather    g_weather;

void loop(void){};  // not used

/*************************************************
* setup
*
* This method is part of the Arduino IDE.
* Initialize the Aclock class (which never returns here)
*
* @param      none
* @return     none
*
*************************************************/
void setup(void)
{ 
  g_aClock.init();
}

/*************************************************
* init
*
* This method initializes the HW 
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::init()
{ 
   _hours   = 0;
   _minutes = 0;
   _seconds = 0;
   _dow     = 0;
   _day     = 0;
   _month   = 0;
   _displayedDay = 0;
   _chimeEnable = true;
   _displayedDow = InvalidDow;  // will update if it is invalid
   
   Serial.begin(9600);
   DebugPrintln();
   DebugPrintln("aClock, The Arduino Clock Project");
   DebugPrintln("---------------------------------");
   g_aClock.initLed();
   g_aClock.initLcd();
   g_timeUtils.init(); 
   g_lcd.WriteObject(GENIE_OBJ_SOUND, 1, 255);   // Set the volume
   g_aClock.mainLoop();
}


/*************************************************
* mainLoop
*
* This method loops forever
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::mainLoop()
{  
   static uint16_t s_updateTempCounter_sec = SecondsToUpdateTemp;
   static uint8_t s_secondCounter = OneMinute_secs;

   g_aClock.updateTimeDisplay();
   g_aClock.updateMonthAndDayDisplay();
   g_aClock.updateDayOfWeekDisplay();

   while(1)
   {
       g_lcd.DoEvents(); // process the queued responses from the display
     
       // run things on a schedule
          
       // every second
        g_aClock.everySecond(); 
          
       // every minute
       s_secondCounter++;
       if (s_secondCounter >= OneMinute_secs)
       {
           s_secondCounter = 0; 
           g_aClock.everyMinute();
       }
          
       // every 15 minutes
       if (s_updateTempCounter_sec++ >= SecondsToUpdateTemp) 
       {
           s_updateTempCounter_sec = 0;
           g_aClock.everyFifteenMinutes();
       }
    
       // once a day
       if ((_hours == 2) && (_minutes == 1) && (_seconds < 10))
       {
           g_aClock.everyDay();         
       }        
       delay(OneSecond);
    }
}

/*************************************************
* lcdEventHandeler
*
* This method handeles events from the Genie LCD display
*
* @param      none
* @return     none
*
*************************************************/
void lcdEventHandeler(void)
{
  genieFrame Event;
  g_lcd.DequeueEvent(&Event);

   // Events triggered from the Events tab of Workshop4 objects
   if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
   {
      if (Event.reportObject.object == GENIE_OBJ_FORM)
      {
        g_aClock._displayedDow = InvalidDow;          // invalidate to force display
        g_aClock.updateDayOfWeekDisplay();
        g_aClock.updateTimeDisplay();
        g_aClock._day = InvalidDow;           // invalidate to force display
        g_aClock.updateMonthAndDayDisplay();
        double temperature;
        bool validTemp = g_weather.getTemperature(temperature);
        if (validTemp == true)
        { 
           g_aClock.displayTempearture(temperature);
        }
      }    
  
      if (Event.reportObject.object == GENIE_OBJ_4DBUTTON)            // handle WINBUTTONs
      {     
         switch (Event.reportObject.index)
         {
            case ButtonChimes:
               g_aClock._chimeEnable = Event.reportObject.data_lsb==0?false:true;
            break;
            
            default:                                                        // unexpected button
            break;
         }
      }
  }
}


/*************************************************
* everySecond
*
* This method gets called once a second
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::everySecond(void)
{ 
   g_aClock.blinkLed();
   g_aClock.updateTimeDisplay();
   g_aClock.updateMonthAndDayDisplay();
   g_aClock.updateDayOfWeekDisplay();
   g_aClock.updateChime();   
   g_webServer.process();                          // let web server run
}

/*************************************************
* everyMinute
*
* This method gets called once a minute
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::everyMinute(void)
{
    DebugPrintln("everyMinuteLoop");
   if (g_wifiShield.checkConnect() == false)
   {
       bool stat = g_aClock.connectToWifi();    
       if (stat == true)
       {
          g_aClock.setRtcWithNtp();        // get NTP time for RTC after a connection
          g_webServer.init();              // initialize web server
       }
   }
}

/*************************************************
* everyFifteenMinutes
*
* This method gets called every 15 minutes
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::everyFifteenMinutes(void)
{
    DebugPrintln("every15MinuteLoop");
   if (g_wifiShield.checkConnect() == true)
   {
      g_aClock.getAndUpdateTempDisplay();
   }
}

/*************************************************
* everyDay
*
* This method gets called once a day
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::everyDay(void)
{
      DebugPrintln("everyDayLoop");
  g_aClock.setRtcWithNtp();
}

/*************************************************
* getAndUpdateTempDisplay
*
* This method updates the displayed temperature
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::getAndUpdateTempDisplay(void)
{      
   bool retVal = g_weather.getWeather();
   if (retVal == true)
   {
     double temperature;
     bool validTemp = g_weather.getTemperature(temperature);
     if (validTemp == true)
     {
        Serial.print("temperature : ");
        Serial.println(temperature);
        g_aClock.displayTempearture(temperature);
     }
   }
}

/*************************************************
* displayTempearture
*
* This method updates the displayed temperature
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::displayTempearture(double temperature)
{   
   uint8_t tempDirectionIndex;
   static double s_aveTemp = 0;
   static bool s_firstTime = true;

   // calculate temperature trend   
  // if (temperature != wifiNamespace::InvalidTemp)
   {
      // se average if first time trough
      if (s_firstTime == true)
      {
        s_firstTime = false;
        s_aveTemp = temperature;
      }
   
      // calculate average over an hour
      s_aveTemp = ((s_aveTemp * (OneMinute_secs - 1)) + temperature) / OneMinute_secs;
      
      if (temperature > s_aveTemp)
      {
        tempDirectionIndex = tempDirectionIndexRising;
      }
      else if (temperature < s_aveTemp)
      {
        tempDirectionIndex = tempDirectionIndexFalling;
      }
      else 
      {
        tempDirectionIndex = tempDirectionIndexSteady;
      }
      
      g_lcd.WriteStr(TempfDirection, (char*)tempDirectionString[tempDirectionIndex]);
      char  tempStr[8];
      sprintf(tempStr, "%dF", (int16_t)(temperature + .5));
      g_lcd.WriteStr(TempfString, tempStr);
      
      // update the graph
      g_lcd.WriteObject(GENIE_OBJ_SCOPE, 0, (temperature + 20) * 1.538);// -20 -> 110, 200 steps

   }
//   else
//   {
//     char nullStr[1] = "";
//      g_lcd.WriteStr(TempfString, nullStr);
//      g_lcd.WriteStr(TempfDirection, nullStr);
//   } 
}

/*************************************************
* updateMonthAndDayDisplay
*
* This method updates the digits on the display
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::updateMonthAndDayDisplay(void)
{
    // set month and day
    if (_day != _displayedDay)
    {
       _displayedDay = _day;
       char  tempStr[14];
       sprintf(tempStr,  "%s %d", MonthString[_month], _day);
       g_lcd.WriteStr(MonthAndDayString, tempStr);                                       
    }
}

/*************************************************
* updateDayOfWeekDisplay
*
* This method updates the digits on the display
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::updateDayOfWeekDisplay(void)
{  
   // set day of week only when it changes (prevents display from flickering)
   if (_dow != _displayedDow)
   {       
      _displayedDow = _dow;
      g_lcd.WriteStr(DayOfWeekString, (char*)DayString[_dow]);                                       
   }
}

/*************************************************
* updateChime
*
* This method updates the digits on the display
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::updateChime(void)
{
  if (_chimeEnable == true)
  {
      static bool g_playingChime = false;

     // check to see if a chime needs to be played
      if ((_minutes == 0) && (g_playingChime == false))
      {
        g_playingChime = true;
        g_lcd.WriteObject(GENIE_OBJ_SOUND, 0, SoundHourPreChime);
        delay(SoundHourPreChimeLenght_ms);
        for (int chimeCount = 0; chimeCount < _hours; chimeCount++)
          {
             g_lcd.WriteObject(GENIE_OBJ_SOUND, 0, SoundHourChime);
             delay(SoundHourChimeLenght_ms);
          }    
      }
      // prevents chime from playing more than once
      if (_minutes != 0)
      {
         g_playingChime = false;
      }
  }
}

/*************************************************
* updateTimeDisplay
*
* This method updates the digits on the display
*
*
* @param      nones 
* @return     none
*
*************************************************/
void Aclock::updateTimeDisplay(void)
{
//   DebugPrintln("updateTimeDisplay");
   g_timeUtils.getTime(&_hours, &_minutes, &_seconds, &_dow, &_day, &_month);
   
   // Set hours digits
   _hours = _hours == 0 ? 12:_hours;      //@todo: add 24 hour mode
   _hours = _hours > 12? _hours -= 12:_hours;
    
   if (_hours > 9)
   {
       g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconHourTens, 0);    // 1
   }
   else
   {
      g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconHourTens, 2);    // Blank
   }
   g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconHourOnes, _hours % 10);    // 0-9

   // Set minutes digits
   g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconMinuteTens, _minutes / 10);    // 0-5
   g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconMinuteOnes, _minutes % 10);    // 0-9
}

/*************************************************
* initLcd
*
* This method initializes the LCD
*
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::initLcd(void)
{
  // initialize the LCD touch screen
  Serial1.begin(9600);                                    // Serial1 
  g_lcd.Begin(Serial1);   
  g_lcd.AttachEventHandler(lcdEventHandeler);            // Attach the LCD Event Handler
  pinMode(LcdResetPin, OUTPUT);                          // Set D4 as output
  digitalWrite(LcdResetPin, 1);                          // Hold LCD in reset
  delay(100);                                            // for 100 ms
  digitalWrite(LcdResetPin, 0);                          // release reset
  delay (3500);                                          // wait for the display after the reset  
  g_lcd.WriteContrast(1);                                // 1 = Display ON, 0 = Display OFF.
  g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconWifi, 0);  // clear wifi icon
}




/*************************************************
* initLed
*
* This method sets the LED pin to an output
*
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::initLed(void)
{
  pinMode(LedPin, OUTPUT); 
}

/*************************************************
* blinkLed
*
* This method blinks the LED
*
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::blinkLed(void)
{
   digitalWrite(LedPin, HIGH);
   delay(1);
   digitalWrite(LedPin, LOW);
}

/************************************************* 
* connectToWifi
*
* This method attempts to connect to the wifi
*
*
* @param      none
* @return     true if connect is successful
*
*************************************************/
bool Aclock::connectToWifi(void)
{
   bool retStat = false;
   g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconWifi, 0);               // clear wifi icon
   DebugPrintln("Attempting to connect to WIFI");   
   if (g_wifiShield.initWifi() == true)
   {
       DebugPrintln("Connected");
       g_lcd.WriteObject(GENIE_OBJ_SOUND, 0, SoundConnect);
       g_lcd.WriteObject(GENIE_OBJ_USERIMAGES, IconWifi, 1);               // set wifi icon
       retStat = true;
    }
    else
    {
        DebugPrintln("Failed to connect");
    }
    return (retStat);
}

/************************************************* 
* setRtcWithNtp
*
* This method sets the RTC to the Network timeserver time
*
*
* @param      none
* @return     none
*
*************************************************/
void Aclock::setRtcWithNtp(void)
{
  DebugPrintln("Attempt to get time from NTP");
   time_t timeUtc;
    bool ntpValid = g_wifiShield.getNtp(&timeUtc);

    if (ntpValid == true)    // set RTC to NTP
    {
    
       DebugPrintln("NTP success");
       DebugPrint("NTP: Month ");
       DebugPrintln(month(timeUtc));
       g_timeUtils.setRtc(timeUtc);
    }
    else
    {
        DebugPrintln("NTP failed");
    }
}

