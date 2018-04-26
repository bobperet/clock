/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/
#include <Time.h>
#include <Adafruit_CC3000_Server.h>
#include "wifiShield.h"
#include "global.h"

// Globals
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(wifiNamespace::Cc3000CsRtc, wifiNamespace::Cc3000Irq, wifiNamespace::Cc3000Vbat, SPI_CLOCK_DIVIDER); // you can change this clock speed but DI
Adafruit_CC3000_Client client;

/*************************************************
* wifiConnect
*
* This method attempts to connect to wifi
*
*
* @param      Index - Index into the list of SSIDs to connect to
* @return     none
*
*************************************************/
bool WifiShield::wifiConnect(const uint8_t Index)
{
  int timeout_sec = 10; // time to wait for DHCP
  cc3000.reboot(0);
  cc3000.begin(); 
  cc3000.deleteProfiles();
  cc3000.connectToAP(wifiNamespace::Ssid[Index].ssid, wifiNamespace::Ssid[Index].passPhrase, wifiNamespace::Ssid[Index].security, 2);
  while (!cc3000.checkDHCP()) {
    delay(1000); 
    if (timeout_sec-- == 0)
    {
      return(false);
    }
  }
  Serial.println();
  cc3000.printIPdotsRev(getIp());
  Serial.println();
  return (true);
}

/*************************************************
* initWifi
*
* This method provides the public interface to call to 
* establish a wifi connection
*
*
* @param      none
* @return     true if successful connection is made
*
*************************************************/
bool WifiShield::initWifi(void)
{
  static uint8_t s_networkIndex = 0;
  bool retStat = wifiConnect(s_networkIndex);
  // save the index of the connected wifiNamespace::Ssid
  g_currentWifiSsid = -1;
  if (retStat == true)
  {
    g_currentWifiSsid = s_networkIndex;
  }
  
  s_networkIndex++;
  if (s_networkIndex > sizeof(wifiNamespace::Ssid) / sizeof(wifiNamespace::ssid_t))
  {
    s_networkIndex = 0;
  }
  return(retStat);
}

/*************************************************
* getNtpTime
*
* This method gets the time from an NTP server
*
*
* @param      none
* @return     time_t structure with the current time
*
*************************************************/
time_t WifiShield::getNtpTime()  //@todo: add a pass/fail return value
{

  uint8_t buf[48];
  unsigned long ip;
  unsigned long startTime;
  unsigned long currentTime = 0;

  if (cc3000.getHostByName("pool.ntp.org", &ip)) 
  {
      static const char PROGMEM
      timeReqA[] = { 227,  0,  6, 236 },
      timeReqB[] = {  49, 78, 49,  52 };
      startTime = millis();
      do 
      {
         client = cc3000.connectUDP(ip, 123);
       } while((!client.connected()) && ((millis() - startTime) < wifiNamespace::ConnectTimeout));

       if (client.connected()) 
       {
           DebugPrintln("Connected to NTP server");

           // Assemble and issue request packet
           memset(buf, 0, sizeof(buf));
           memcpy_P( buf    , timeReqA, sizeof(timeReqA));
           memcpy_P(&buf[12], timeReqB, sizeof(timeReqB));
           client.write(buf, sizeof(buf));

           memset(buf, 0, sizeof(buf));
           startTime = millis();
           while ((!client.available()) && ((millis() - startTime) < wifiNamespace::ResponseTimeout));
           if (client.available()) 
           {
               client.read(buf, sizeof(buf));
               //@todo: Validate data
               currentTime = (((unsigned long)buf[40] << 24) |
                              ((unsigned long)buf[41] << 16) |
                              ((unsigned long)buf[42] <<  8) |
                               (unsigned long)buf[43]) - 2208988800UL;
           }
           client.close();
       }
  }
  return currentTime;
}

/*************************************************
* getNtp
*
* This method gets the time from an NTP server
*
*
* @param      none
* @return     none
*
*************************************************/
bool WifiShield::getNtp(time_t* timeUtc_ptr)
{
  bool retStatus = false;
     
  *timeUtc_ptr = getNtpTime();
  // check to see if the NTP server responded
  if (hour(*timeUtc_ptr) != 0)
  {
    retStatus = true; 
  }
  return(retStatus);
}

/*************************************************
* getIp
*
* This method returns the IP address of this device
*
*
* @param      none
* @return     uint32_t containing this IP address
*
*************************************************/
uint32_t WifiShield::getIp(void)
{
  uint32_t retip;
  uint32_t netmask;
  uint32_t gateway;
  uint32_t dhcpserv;
  uint32_t dnsserv;

  cc3000.getIPAddress(&retip, &netmask, &gateway, &dhcpserv, &dnsserv);
  return(retip);
}

/*************************************************
* checkConnect
*
* This method is the public interface to check the status
* of the wifi connection
*
*
* @param      none
* @return     true if connected to wifi
*
*************************************************/
bool WifiShield::checkConnect(void)
{
   Serial.println("Checking WIFI connection.");
   bool stat =  cc3000.checkConnected();
   if (stat == true)
   {
     Serial.println("Connected.");
   }
   else
   {
     Serial.println("Not connected.");
   }
   
   return(stat);
}

/*************************************************
* ping DEBUG
*
* This method pings the gateway
*
*
* @param      none
* @return     none
*
*************************************************/
uint16_t WifiShield::ping(uint8_t count)
{
   uint32_t ip = 0xC0A80001;  //192.168.0.1
   uint16_t numSuccess = cc3000.ping(ip, count, 500, 64);  // address, count, msTimeout, bytes
   return (numSuccess);
}

/*************************************************
* getSsid
*
* This method returns the current ssid
*
*
* @param      none
* @return     pointer to a string containing the name of the SSID
*
*************************************************/
const char* WifiShield::getSsid(void)
{
  if (g_currentWifiSsid >=0)
  {
      return(wifiNamespace::Ssid[g_currentWifiSsid].ssid);
  }
  else return("No Ssid");
}

/*************************************************
* getXml
*
* This method returns the value of the specified XML string
* 
*
* @param      XML to locate (i.e. <temp_f> will return 21.0 if the XML contains <temp_f>21.0</temp_f>)
* @return     char pointer th the located XML (null if not found)
*
*************************************************/
bool WifiShield::getXml(const char* xmlToFind, char* inputBuffer, char* outputBuffer)
{
   bool returnStatus = false;
    char charRead;
    bool xmlFound = false;
    uint16_t j = 0;
    uint16_t i = 0;
    uint16_t index;
      
    for (index = 0; index < strlen(inputBuffer); index++)
    {
        if (inputBuffer[index] == xmlToFind[j])
        {
           j++;                         // character matched, bump index to next one
           if (j >= strlen(xmlToFind))  // check to see if the match is complete
           {
             xmlFound = true;
             break;
           }
        }
        else
        {
           j = 0;  // restart string search
        }
     }
      
   // Get the XML value
   if (xmlFound == true)
   {
       for (index = index + 1; index < strlen(inputBuffer); index++)
       {
          charRead = inputBuffer[index];
          if (charRead == '<')
           {
              outputBuffer[i] = 0;
              returnStatus = true;
              break;    // got complete string
           }
           outputBuffer[i++] = charRead;
       }
    }
    else
    {
        outputBuffer[0] = 0;  // clear return string
    }

     return (returnStatus);
}


/*************************************************
* getXmlPage
*
* This method returns the page read from
* the specified URL
* 
*
* @param      char *server - The name of the server to access
* @param      char *URL - The location of the XML on the server
* @param      char* page - Points to a buffer to store the retrieved XML
* @return     bool - true if successful read
*
*************************************************/
bool WifiShield::getXmlPage(const char *server, const char *url, char *page)
{
   bool returnStatus = false;
   const uint16_t port = 80;
   uint16_t index = 0;
   char charRead;
 
   if (client.connect(server, port) == 1) 
   {
       DebugPrint("Connected to ");
       DebugPrintln(server);
      
       // Make an HTTP request:
       client.print("GET ");
       client.print(url);
       client.println(" HTTP/1.1");
       
       client.print("host: ");
       client.println(server);
       
       client.println("User-Agent: aClock");
       client.println();
        
       const uint16_t IdleTimeout_MS = 3000;  
        // Read data until either the connection is closed, or the idle timeout is reached.  
       unsigned long lastRead = millis();
       while(client.connected() && (millis() - lastRead < IdleTimeout_MS))
       {
         if (client.available() != 0)
         {
             charRead = client.read();         
             page[index++] = charRead;
             if (index >= 4000)
             {
                DebugPrintln("XML is too big\n");
                index=0;
                break;
             }
          }
       }
       page[index] = 0;  // null terminate string
       client.close();   //  close connection to server
       client.close();   //  close connection to server
       if (index > 0)
       { 
           returnStatus = true;
       }
   }
   return(returnStatus);
}
