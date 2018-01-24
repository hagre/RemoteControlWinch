#include "definitions.h"
#include "WS2812Operator.h"

//#define DEBUG_SERIAL_WS2812


WS2812Operator::WS2812Operator (WS2812& myWS2812): _LED (myWS2812) {
  _LED.setOutput(WS2812_DATA_PIN);
  _allOff.r = 0;
  _allOff.g = 0;
  _allOff.b = 0;
  _allOn.r = 255;
  _allOn.g = 255;
  _allOn.b = 255;
  for (int i = 0; i < WS2812_NUMBER_OF_LEDS; i++){
    _LEDMode [i] = 0;
    _LEDStatus [i] = 0;
    _LEDColor [i] = _allOn;
  }
}

int WS2812Operator::operate (bool now = false){
  int done = 0;
  #ifdef DEBUG_SERIAL_WS2812
    Serial.print ("Operate WS2812 with now = ");
    Serial.println (now);
    Serial.println (millis ());
  #endif
  if (millis () > _nextOperation){
    now = true;
    #ifdef DEBUG_SERIAL_WS2812
      Serial.print (" Time to set LEDS ");
    #endif
    _nextOperation = millis() + _speed;
    for (int i = 0; i < WS2812_NUMBER_OF_LEDS; i++){
      switch (_LEDMode [i]) {
        case 0: //off
          _LEDStatus [i] = 0;
          _LED.set_crgb_at(i, _allOff);
          break;
        case 1: //on
          _LEDStatus [i] = 1;
          _LED.set_crgb_at(i, _LEDColor [i]);
          break;
        case 2: //blink
          if (_LEDStatus [i] == 0){
            _LEDStatus [i] = 1;
            _LED.set_crgb_at(i, _LEDColor [i]);
          }
          else if (_LEDStatus [i] == 1){
            _LEDStatus [i] = 0;
            _LED.set_crgb_at(i, _allOff);
          }
          break;
        case 3: // Blink start with ON
          _LEDStatus [i] = 1;
          _LED.set_crgb_at(i, _LEDColor [i]);
          _LEDMode [i] = 2;
          break;
        case 4: // Blink start with OFF
          _LEDStatus [i] = 0;
          _LED.set_crgb_at(i, _allOff);
          _LEDMode [i] = 2;
          break;
      }
    }
  }
  if (now == true){
    #ifdef DEBUG_SERIAL_WS2812
      Serial.print (" Time to synch/send to LEDS ");
    #endif
    _LED.sync();
    done = 1;
  }
  return done;
}

void WS2812Operator::setSpeed (unsigned int newSpeed){
  _speed = newSpeed;
}

void WS2812Operator::setMode (int LEDNr, int newMode){
  _LEDMode [LEDNr] = newMode;
}

void WS2812Operator::setColor (int LEDNr, cRGB& pnewColor){
  _LEDColor [LEDNr] = pnewColor;
}

void WS2812Operator::setAll (int newMode, cRGB& pnewColor){
  if (newMode == 0 || newMode == 1 || newMode == 2){
    for (int i = 0; i < WS2812_NUMBER_OF_LEDS; i++){
      setMode (i, newMode);
      setColor (i, pnewColor);
    }
  }
  else if (newMode == 3){
    for (int i = 0; i < WS2812_NUMBER_OF_LEDS; i = i + 2){
      setMode (i, 3);
      setColor (i, pnewColor);
    }
    for (int i = 1; i < WS2812_NUMBER_OF_LEDS; i = i + 2){
      setMode (i, 4);
      setColor (i, pnewColor);
    }
  }
  else if (newMode == 4){
    for (int i = 0; i < WS2812_NUMBER_OF_LEDS; i = i + 2){
      setMode (i, 4);
      setColor (i, pnewColor);
    }
    for (int i = 1; i < WS2812_NUMBER_OF_LEDS; i = i + 2){
      setMode (i, 3);
      setColor (i, pnewColor);
    }
  }
}
