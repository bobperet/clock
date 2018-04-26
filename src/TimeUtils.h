/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#ifndef _timeUtils_h
#define _timeUtils_h

#include <arduino.h>
#include <time.h>
//#include <RTClib.h>          // The RTC_DS1307 battery backuped RTC

namespace timeUtilsNamespace
{
  const uint32_t TimezoneOffset = -4;       // @todo: this should be settable by the user
  const uint32_t OneHour_secs   = 3600;
}
class TimeUtils
{
   public:
      TimeUtils(){};
      ~TimeUtils(){};
      void init(void);
      void adjustForDst(void);
      void setRtc(time_t timeUtc);
      void getTime(uint8_t *_hours, uint8_t *_minutes, uint8_t *_seconds, uint8_t *dow, uint8_t *_day, uint8_t *_month);
      bool getTimeToUpdateFlag()
      {
        if (_timeUpdate == true)
        {          
            _timeUpdate = false;
            return (true);
        }
        return(false);
    }
      static bool _timeUpdate;
      
   private:
};

#endif
