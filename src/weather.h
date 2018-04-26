/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#ifndef WEATHER_H
#define WEATHER_H

namespace weatherNamespace
{
  static const uint8_t TepmeratureUnitC = 0;
  static const uint8_t TepmeratureUnitF = 1;
  static const uint8_t TepmeratureUnitK = 2;

  static const uint8_t PressureUnitsIn = 0;
  static const uint8_t PressureUnitsMm = 1;
  static const uint8_t PressureUnitsMb = 2;

};

class Weather
{
  public:
    Weather();
    ~Weather(){};
    bool getTemperature(double &temperature);
    bool getPressure(double pressure);
    bool getWeatherText(char* weatherText);
    bool getWeather(void);
  
  private:
    bool _weatherValid;
    uint8_t _tempertureUnits;
    uint8_t _pressureUnits;
    
    double _temperature_C;
    double _pressure_in;
    char _weatherText[20];
};
extern Weather g_weather;
#endif
