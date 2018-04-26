/*
*   COPYRIGHT 2014, 2015
*   Bob Peret
*
*   Contains confidential and proprietary information which
*   may not be copied, disclosed or used by others except as expressly
*   authorized in writing by Bob Peret.
*/

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
#include "webServer.h"
#include "weather.h"
#include "wifiShield.h"

using namespace webServerNamespace;

Adafruit_CC3000_Server httpServer(WebServerPort);

/*************************************************
* init
*
* This method  
*
* @param      none
* @return     none
*
*************************************************/
void WebServer::init(void)
{
  httpServer.begin();    // Start listening for connections
}

/*************************************************
* process
*
* This method  
*
* @param      none
* @return     none
*
*************************************************/
void WebServer::process(void)
{
  if (g_wifiShield.checkConnect() == false)
  {
    return;
  }
  
  static uint8_t  s_tempF = 70;  // just for testing
  static uint16_t s_bufferIndex = 0;
  static uint8_t  s_buffer[IncomingDataBufferSize + 1];// @todo: why +1?
  static char     s_action[MaxTokenLength+1];
  static char     s_path[MaxPathLength+1];

  Adafruit_CC3000_ClientRef client = httpServer.available();    // check to see if there is a connection from a client
  if (client) 
  {
    // Process this request until it completes or times out.
    // Note that this is explicitly limited to handling one request at a time!

    // Clear the incoming data buffer and point to the beginning of it.
    s_bufferIndex = 0;
    memset(&s_buffer, 0, sizeof(s_buffer));
    
    // Clear action and path strings.
    memset(&s_action, 0, sizeof(s_action));
    memset(&s_path,   0, sizeof(s_path));

    // Set a timeout for reading all the incoming data.
    unsigned long endtime = millis() + IncomingRequestTImeout_ms;
    
    // Read all the incoming data until it can be parsed or the timeout expires.
    bool parsed = false;
    while (!parsed && (millis() < endtime) && (s_bufferIndex < IncomingDataBufferSize)) {
      if (client.available()) {
        s_buffer[s_bufferIndex++] = client.read();
      }
      parsed = parseRequest(s_buffer, s_bufferIndex, s_action, s_path);
    }

    // Handle the request if it was parsed.
    if (parsed) {
      // Check the action to see if it was a GET request.
      if (strcmp(s_action, "GET") == 0) {
        // Respond with the path that was accessed.
        // First send the success response code.
        client.fastrprintln("HTTP/1.1 200 OK");
        // Then send a few headers to identify the type of data returned and that
        // the connection will not be held open.
    //    client.fastrprintln(F("Content-Type: text/richtext"));
        client.fastrprintln("Connection: close");
        client.fastrprintln("Server: aClock");
        // Send an empty line to signal start of body.
        client.fastrprintln("");
        // Now send the response data.
        client.fastrprintln("<html>");
        client.fastrprintln("<head><META HTTP-EQUIV='refresh' CONTENT='15'></head>");
        client.fastrprintln("<body>");
        client.fastrprintln("<b><font color='blue' size = '6'>aClock</font></b><hr>");
        client.fastrprintln("<table border=1><tr>");
        client.fastrprintln("<td>Temperature</td>");
        client.fastrprintln("<td>");
        char str[8];
        double temperature = 0.0;
        g_weather.getTemperature(temperature);
   //     sprintf(str,"%d", (int16_t)(temperature + .5));
        dtostrf(temperature, 4, 2, str);
        Serial.println(str);
        client.fastrprintln(str);
        client.fastrprintln("</td></tr></table>");
        client.fastrprintln("</body>");
        client.fastrprintln("</html>");
        client.fastrprintln(" ");
      
//        client.fastrprint(F("You accessed path: ")); client.fastrprintln(path);
      }
      else {
        // Unsupported action, respond with an HTTP 405 method not allowed error.
        client.fastrprintln("HTTP/1.1 405 Method Not Allowed");
        client.fastrprintln("");
      }
    }

    // Wait a short period to make sure the response had time to send before
    // the connection is closed (the CC3000 sends data asyncronously).
    delay(100);

    // Close the connection when done.
    client.close();
  }
}

// Return true if the buffer contains an HTTP request.  Also returns the request
// path and action strings if the request was parsed.  This does not attempt to
// parse any HTTP headers because there really isn't enough memory to process
// them all.
// HTTP request looks like:
//  [method] [path] [version] \r\n
//  Header_key_1: Header_value_1 \r\n
//  ...
//  Header_key_n: Header_value_n \r\n
//  \r\n
bool WebServer::parseRequest(uint8_t* buf, int bufSize, char* action, char* path) 
{
  // Check if the request ends with \r\n to signal end of first line.
  if (bufSize < 2)
    return false;
  if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') {
    parseFirstLine((char*)buf, action, path);
    return true;
  }
  return false;
}

// Parse the action and path from the first line of an HTTP request.
void WebServer::parseFirstLine(char* line, char* action, char* path) 
{
  // Parse first word up to whitespace as action.
  char* lineaction = strtok(line, " ");
  if (lineaction != NULL)
    strncpy(action, lineaction, MaxTokenLength);
  // Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, " ");
  if (linepath != NULL)
    strncpy(path, linepath, MaxPathLength);
}

