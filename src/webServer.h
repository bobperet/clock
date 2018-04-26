/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#ifndef _webServer_h
#define _webServer_h

#include <Adafruit_CC3000_Server.h>
#include <Adafruit_CC3000.h>

namespace webServerNamespace
{
  const uint16_t WebServerPort             = 80;      
  const uint16_t MaxPathLength             = 64;                                  // Maximum length of the HTTP request path that can be parsed
  const uint16_t MaxTokenLength            = 10;                                  // Maximum length of the HTTP token that can be parsed
  const uint16_t IncomingDataBufferSize    = MaxTokenLength + MaxPathLength + 20; // Size of buffer for incoming requests
  const uint16_t IncomingRequestTImeout_ms = 500;                                 // Incoming request timeout
}

class WebServer
{
   public:
      WebServer(){};
      ~WebServer(){};
      void init(void);
      void process(void);

   private:
      bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path);
      void parseFirstLine(char* line, char* action, char* path);

};
extern WebServer g_webServer;
extern Adafruit_CC3000 cc3000;
#endif
