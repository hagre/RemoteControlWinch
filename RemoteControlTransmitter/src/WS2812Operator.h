
#ifndef WS2812Operator_h
#define WS2812Operator_h

#include "definitions.h"
#include <WS2812.h>

#include <Arduino.h>

//to write in main.cpp
//
//#include <WS2812.h>
//cRGB red, blue, green, yellow;
//WS2812 LED(WS2812_NUMBER_OFF_LEDS);
//WS2812Operator myWS2812LEDs (&LED);

class WS2812Operator {

public:
  WS2812Operator (WS2812& myWS2812);
  int operate (bool now = false); //call for update and sync with leds according to speed. When needed now=true to force update
  void setSpeed (unsigned int newSpeed); //ms to update blink
  void setMode (int LEDNr, int newMode); // mode 0=off, mode 1=on, mode 2=blink, mode 3=blink to start with on, 3=blink to start with off
  void setColor (int LEDNr, cRGB& pnewColor); //eg. cRGB red;  red.r = 255;   red.b = 0;   red.g = 0;
  void setAll (int newMode, cRGB& pnewColor); // set all to one mode and color

private:
  unsigned int _speed = 0;
  unsigned long _nextOperation;
  cRGB _allOff, _allOn;
  WS2812& _LED;
  int _LEDMode [WS2812_NUMBER_OF_LEDS];
  cRGB _LEDColor [WS2812_NUMBER_OF_LEDS];
  int _LEDStatus [WS2812_NUMBER_OF_LEDS];
};
#endif //WS2812Operator_h
