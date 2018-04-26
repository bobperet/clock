/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/
#include <arduino.h>
#include "global.h"
#include "weather.h"
#include "wifiShield.h"

/*************************************************
* Weather
*
* This method initializes class variables
*
*
* @param      none
* @return     none
*
*************************************************/
Weather::Weather()
{
   _tempertureUnits = weatherNamespace::TepmeratureUnitF;
   _pressureUnits = weatherNamespace::PressureUnitsIn;
   _weatherValid = false;
}

/*************************************************
* getTemperature
*
* This method gets the current temperature (whole number)
*
*
* @param      none
* @return     bool true = success
*
*************************************************/
bool Weather::getTemperature(double &temperature)
{
  if (_weatherValid == true)
  {
    switch(_tempertureUnits)
    {
      case weatherNamespace::TepmeratureUnitC:
         temperature = _temperature_C;
      break;

      case weatherNamespace::TepmeratureUnitF:
         temperature = (_temperature_C * 1.8) + 32;
      break;

      case weatherNamespace::TepmeratureUnitK:
         temperature = _temperature_C - 273.15;
      break;

      default:
        printf("***Error: temperature weatherUnit is invalid\n");
      break;
    }
  }
  
  return(_weatherValid);  
}
 
/*************************************************
* getPresure
*
* This method gets the current pressure in
* whatever units were selected
*
*
* @param      none
* @return     none
*
*************************************************/
bool Weather::getPressure(double pressure)
{
  bool returnStatus = false;
  
  if (_weatherValid == true)
  {
    returnStatus = true;
    switch(_pressureUnits)
    {
      case weatherNamespace::PressureUnitsIn:
         pressure = _pressure_in;
      break;

      case weatherNamespace::PressureUnitsMm:
         pressure = _pressure_in * 25.4;
      break;
      
      case weatherNamespace::PressureUnitsMb:
         pressure = _pressure_in * 33.864;
      break;
      
      default:
        printf("***Error: pressure weatherUnit is invalid\n");
      break;
    }
  }
  
  return(returnStatus);  
}

/*************************************************
* Weather
*
* This method initializes class variables
*
*
* @param      none
* @return     none
*
*************************************************/
bool Weather::getWeatherText(char* weatherText)
{
   bool returnStatus = false;
  
  if (_weatherValid == true)
  {
    returnStatus = true;
    weatherText = _weatherText;
  }
  
  return(returnStatus);  
}

/*************************************************
* getWeather
*
* This method returns the current weather from NOAA.gov
*
*
* @param      none
* @return     none
* @todo:      This should fill in a structure with all of theweather information
*
*************************************************/
bool Weather::getWeather(void)
{
  _weatherValid = false;
   static uint32_t s_count = 0;
   DebugPrint("Getting weather.");
   DebugPrintln(s_count++);

   bool retVal = false;
   const char server[] = "w1.weather.gov";
   const char url[] = "/xml/current_obs/KMHT.xml";
   char page[4000];
   const char xmlWeather[]    = "<weather>"; // <temp_f>21.0</temp_f>
   const char xmlTempC[]      =  "<temp_c>"; 
   const char xmlPressureIn[] =  "<pressure_in>"; 
   char weather[80];
   char pressureIn [8];
   char tempC[8];
  
   retVal = g_wifiShield.getXmlPage(server, url, page);
   if (retVal == false)
   {
     return(retVal);
   }
   
   retVal = g_wifiShield.getXml(xmlWeather, page, weather);
   if (retVal == false)
   {
     return(retVal);
   }
   
   retVal = g_wifiShield.getXml(xmlTempC, page, tempC);
   if (retVal == true)
   {
     sscanf(tempC, "%f", &_temperature_C); 
   }
   else
   {
     return(retVal);
   }
   
   retVal = g_wifiShield.getXml(xmlPressureIn, page, pressureIn);
   if (retVal == false)
   {
     return(retVal);
   }
   _temperature_C = atof(tempC);
   Serial.println(_temperature_C);
   _pressure_in = atof(pressureIn);
   strcpy(_weatherText, weather);
   
   DebugPrint ("Tepmerature: ");
   DebugPrintln(tempC);
   
   DebugPrint ("Pressure: ");
   DebugPrintln(pressureIn);
   
   DebugPrint ("Weather: ");
   DebugPrintln(weather);
   _weatherValid = retVal;
   return (retVal);
}


