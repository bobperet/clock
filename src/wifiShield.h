/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#ifndef _wifiShield_h
#define _wifiShield_h

#include <Adafruit_CC3000_Server.h>
#include <Adafruit_CC3000.h>

namespace wifiNamespace
{
    typedef struct
   {
     const char *ssid;
     const char *passPhrase;
     const char security;
   } ssid_t;

   const int16_t  InvalidTemp     = -999;
   const int16_t  Cc3000Irq       = 3;                               // WiFi uses SCK (D13), MISO (D12), MOSI (D11), CS for CC3000 (D10), VBAT_EN (D5), CS for SD Card (D4), IRQ (D3)
   const int16_t  Cc3000Vbat      = 5;
   const int16_t  Cc3000CsRtc     = 10;
   const uint32_t ConnectTimeout  = 10000;                     // Max time to wait for server connection
   const uint32_t ResponseTimeout = 10000;                     // Max time to wait for data from server
   const char     PassPhrase[]    = "feedabadb0bdeadbeef0123456";    // @todo: this needs t be kept on the SD card and an interface created for the user to enter it
   const ssid_t Ssid[] = {
                            {"Bobnet",   PassPhrase, WLAN_SEC_WPA2},
                            {"Bobnet-B", PassPhrase, WLAN_SEC_WPA2},
                            {"Bobnet-g", PassPhrase, WLAN_SEC_WPA2}
                         };
}

class WifiShield
{
   public:
      WifiShield(){};
      ~WifiShield(){};
      
      uint32_t LastPolledTime;   // Last value retrieved from time server
      uint32_t sketchTime;      // CPU milliseconds since last server query
      int8_t g_currentWifiSsid;

      bool        initWifi(void);
      time_t      getNtpTime();
      bool        getNtp(time_t* timeUtc_ptr);
      uint32_t    getIp(void);
      bool        checkConnect(void);
      uint32_t    myPing();
      const char* getSsid(void);
      uint16_t    ping(uint8_t count);
      int16_t     getWeather(void);
      bool        getXmlPage(const char *server, const char *url, char *page);
      bool        getXml(const char* xmlToFind, char* inputBuffer, char* outputBuffer);

   private:
      bool        wifiConnect(const uint8_t ssid);

};
extern WifiShield g_wifiShield;
#endif
