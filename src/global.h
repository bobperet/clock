#ifndef GLOBAL_H
#define GLOBAL_H

// Define "DEBUG" for DebugPrintln to output to serial port
#define DEBUG
#ifdef DEBUG
  #define DebugPrintln Serial.println
  #define DebugPrint Serial.print
#else
  #define DebugPrintln //
  #define DebugPrint //
#endif
#endif
