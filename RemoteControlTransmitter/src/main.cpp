/**
 * REMOTE CONTROL TRANSMITTER FOR AGRICULTURAL WICH by hagre 2018
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

 // REMOTE CONTROL TRANSMITTER for an agricultural winch made with an Arduino Nano (5v) and an
 // MPU9250 sensor connected via I2C. 6 switches ("multifunktion- ,pull- ,push- , start-, stop- and emergency button")
 // Indications with WS2812 LEDs. Lipo Battery, some step up/down power regulators and chargin unit, logiclevel p-chanal MOSFET to
 // switch supply power on anf off via the arduino. The MPU9250 ist to detect an possible unconsciousness of the user.
 // Differnet emergency levels and a stabile and relaible RF connection including aknolegement of the receiver witch a 433mhz
 // RF69 module.
 //
 //  --------------------------------------------------------------------
 //  You will need the "LightWS2812" library.                           !
 //  It can be found under "ID159" in the PlatformIO library manager.   !
 //  https://github.com/cpldcpu/light_ws2812                            !
 //  --------------------------------------------------------------------
 //  --------------------------------------------------------------------
 //  You will need the "MPU9250_asukiaaa" library.                      !
 //  It can be found under "ID1751" in the PlatformIO library manager.  !
 //  https://github.com/asukiaaa/MPU9250_asukiaaa                       !
 //  --------------------------------------------------------------------
 //  --------------------------------------------------------------------
 //  You will need the "RadioHead" library.                             !
 //  It can be found under "ID124" in the PlatformIO library manager.   !
 //  http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.82.zip!
 //  --------------------------------------------------------------------
 //  --------------------------------------------------------------------
 //  You will need the "Polymorphic Buttons" library.                   !                    !
 //  It can be found under "ID1899" in the PlatformIO library manager.  !
 //  https://github.com/JCWentzel/PolymorphicButtons                    !
 //  ==> at the time of this posting my push request was not merged,    !
 //  ==> so i had to add the modified version of this library           !
 //  ==> it can also be found at github hagre/PolymorphicButtons        !
 //  --------------------------------------------------------------------

 #include <Arduino.h>

 #include <definitions.h>
 //#define DEBUG_SERIAL
 //#define DEBUG_SERIAL_BAT
 //#define DEBUG_SERIAL_MAIN
 //#define DEBUG_IMU
 #define IMU

 #define LIGHTFACTOR 50 //100%

 #include <RHReliableDatagram.h>
 #include <RH_RF69.h>
 #include <SPI.h>

 #include <MPU9250_asukiaaa.h>
 MPU9250 mySensor;

 #include <RFTransceiver.h>
 RH_RF69 myRFdriver(RFTRANSCEIVER_NSS, RFTRANSCEIVER_DIO);
 RHReliableDatagram myRFManager(myRFdriver, RF_ADDRESS);
 RFTransceiver myRF(myRFdriver, myRFManager);


 #include <WS2812.h>
 cRGB red, blue, green, yellow;
 WS2812 LED(WS2812_NUMBER_OF_LEDS);
 #include "WS2812Operator.h"
 WS2812Operator myWS2812LEDs (LED);

 #include "PMButton.h"
 PMButton buttonEmergency (EMERGENCY_BUTTON_PIN);
 PMButton buttonPull (PULL_BUTTON_PIN);
 PMButton buttonPush (PUSH_BUTTON_PIN);
 PMButton buttonSecondfunction (SECONDFUNCTION_BUTTON_PIN);
 PMButton buttonStart (START_PLUS_BUTTON_PIN);
 PMButton buttonStop (STOP_MINUS_BUTTON_PIN);

 int lastStateRunningRemote = STATUS_REMOTE_INIT;
 int lastStateRunningEmergency = STATUS_EMERGENCY_NONE;
 int lastStateRunningButton = STATUS_BUTTON_NONE;


 unsigned long timeAtShutoff = 0;
 unsigned long timeRemainingToShutoff = SWITCH_OFF_REMAINING_TIME / 1000;
 unsigned long timingNextVoltmeter = 0;
 unsigned long deadManTimeWillBe = TIME_EMERGENCY_DEADMAN;
 long batteryVoltage = 0;
 int transceivingProblemMaker = -1;
 int receivedRPM = 0;
 int receivedWinshState = STATUS_WINCH_UNKNOWN;
 int receivedWinshStateEmergency = STATUS_EMERGENCY_UNKNOWN;
 int receivedWinshStateMotor = STATUS_MOTOR_UNKNOWN;
 int receivedWinshStatePowertransmission = STATUS_POWERTRANSMISSION_UNKNOWN;
 int receivedWinshStateTilt = STATUS_WINCH_TILT_UNKNOWN;

 void executeShutdown (){
   digitalWrite (MOSFET_POWER_PIN, LOW);
 }

 void checkLEDtoBe (){
   //check Buttons
   if (lastStateRunningRemote == STATUS_REMOTE_EMERGENCY){ // WS2812_REMOTE_EMERGENCY_BUTTON_LED = red on pressed, and blink in case of emergency (when actually not pressed)
     myWS2812LEDs.setMode (WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED, WS2812_BLINK); // WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED = blue on pressed, and blink in case of emergency (when actually not pressed)
     myWS2812LEDs.setColor (WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED, WS2812_RED);
     if (buttonEmergency.isPressed()){
       myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_BUTTON_LED, WS2812_ON);
       myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_BUTTON_LED, WS2812_RED);
     }
     else {
       myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_BUTTON_LED, WS2812_BLINK);
       myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_BUTTON_LED, WS2812_RED);
     }
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_BUTTON_LED, WS2812_OFF);
     if (buttonSecondfunction.isPressed()){ // WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED = blue on pressed, and blink in case of emergency (when actually not pressed)
       myWS2812LEDs.setMode (WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED, WS2812_ON);
       myWS2812LEDs.setColor (WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED, WS2812_BLUE);
       if (batteryVoltage >= 90){ // check the battery status on extra leds if secondary button is pressed
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 80 && batteryVoltage < 90){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_YELLOW);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 70 && batteryVoltage < 80){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 60 && batteryVoltage < 70){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_YELLOW);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 50 && batteryVoltage < 60){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 40 && batteryVoltage < 50){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_YELLOW);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 30 && batteryVoltage < 40){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_GREEN);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 20 && batteryVoltage < 30){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_YELLOW);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_GREEN);
       }
       else if (batteryVoltage >= 10 && batteryVoltage < 20){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_BLINK);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_YELLOW);
       }
       else if (batteryVoltage >= 5 && batteryVoltage < 10){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_BLINK);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_RED);
       }
       else if (batteryVoltage < 5){
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_OFF);
         myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_ON);
         myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_RED);
       }
     }
     else {
       myWS2812LEDs.setMode (WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED, WS2812_OFF);
       myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_OFF);
       myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_OFF);
       myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_OFF);
       myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_OFF);
       myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_OFF);
     }
   }

   //push short
   if (lastStateRunningButton == STATUS_BUTTON_PUSHSHORT){ //Push button short = blue
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_BUTTON_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_PUSH_BUTTON_LED, WS2812_BLUE);
   }
   //push long
   else if (lastStateRunningButton == STATUS_BUTTON_PUSHLONG){ //Push button long = blue blink
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_BUTTON_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_PUSH_BUTTON_LED, WS2812_BLUE);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_BUTTON_LED, WS2812_OFF);
   }

   //pull
   if (lastStateRunningButton == STATUS_BUTTON_PULL){ //pull button = yellow
     myWS2812LEDs.setMode (WS2812_REMOTE_PULL_BUTTON_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_PULL_BUTTON_LED, WS2812_YELLOW);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_PULL_BUTTON_LED, WS2812_OFF);
   }

   //motorstart
   if (lastStateRunningButton == STATUS_BUTTON_MOTORSTART){ //motor start button = green blink
     myWS2812LEDs.setMode (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_GREEN);
   }
   //high tourque
   else if (lastStateRunningButton == STATUS_BUTTON_HIGHTORQUE){ //motor hightorque button = green
     myWS2812LEDs.setMode (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_GREEN);
   }
   //motorstop
   else if (lastStateRunningButton == STATUS_BUTTON_MOTORSTOP){ //motor stop button = red blink
     myWS2812LEDs.setMode (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_RED);
   }
   //low tourque
   else if (lastStateRunningButton == STATUS_BUTTON_LOWTORQUE){ //motor lowtorque button = red
     myWS2812LEDs.setMode (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_START_STOP_BUTTON_LED, WS2812_OFF);
   }


   //Statusindication Leds
   //Emergency
   if (receivedWinshStateEmergency == STATUS_EMERGENCY_DEADMAN || receivedWinshStateEmergency == STATUS_EMERGENCY_EXTERN || receivedWinshStateEmergency == STATUS_EMERGENCY_LOCAL ){ //red blink
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStateEmergency == STATUS_EMERGENCY_STOP){ //red
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStateTilt == STATUS_WINCH_TILT_EMERGENCY_CROSS || receivedWinshStateTilt == STATUS_WINCH_TILT_EMERGENCY_LENGTH){ //yellow blink
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_YELLOW);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_OFF);
   }

   //Tilt
   if (receivedWinshStateTilt == STATUS_WINCH_TILT_ALARM_CROSS || receivedWinshStateTilt == STATUS_WINCH_TILT_ALARM_LENGTH){ //yellow blink
       myWS2812LEDs.setMode (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_BLINK);
       myWS2812LEDs.setColor (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_YELLOW);
   }
   else if (receivedWinshStateTilt == STATUS_WINCH_TILT_EMERGENCY_CROSS || receivedWinshStateTilt == STATUS_WINCH_TILT_EMERGENCY_LENGTH){ //red blink
     myWS2812LEDs.setMode (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_OFF);
   }

   //battery
   if (batteryVoltage >= 50){
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_GREEN);
   }
   else if (batteryVoltage >= 30 && batteryVoltage < 50){
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_YELLOW);
   }
   else if (batteryVoltage >= 10 && batteryVoltage < 30){
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_RED);
   }
   else if (batteryVoltage < 10){
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_SOLO, WS2812_RED);
   }

   //PULL
   if (receivedWinshState == STATUS_WINCH_PULL){ //blue
     myWS2812LEDs.setMode (WS2812_REMOTE_PULL_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_PULL_STATUS_LED, WS2812_BLUE);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_PULL_STATUS_LED, WS2812_OFF);
   }
   //PUSH
   if (receivedWinshState == STATUS_WINCH_PUSH){ //green
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_PUSH_STATUS_LED, WS2812_GREEN);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_STATUS_LED, WS2812_OFF);
   }

   //powertransmission
   if (receivedWinshStatePowertransmission == STATUS_POWERTRANSMISSION_STOP){ //red
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStatePowertransmission == STATUS_POWERTRANSMISSION_STARTING){ //yellow blink
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_YELLOW);
   }
   else if (receivedWinshStatePowertransmission == STATUS_POWERTRANSMISSION_RUN){ //green
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_GREEN);
   }
   else if (receivedWinshStatePowertransmission == STATUS_POWERTRANSMISSION_STOPING){ //red
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStatePowertransmission == STATUS_POWERTRANSMISSION_UNKNOWN){ //off
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_OFF);
   }
   else { //off
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_OFF);
   }

   //Motor
   if (receivedWinshStateMotor == STATUS_MOTOR_STOP){ //red
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStateMotor == STATUS_MOTOR_STARTING){ //yellow blink
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_YELLOW);
   }
   else if (receivedWinshStateMotor == STATUS_MOTOR_RUNLOW || receivedWinshStateMotor == STATUS_MOTOR_SPEEDUP || receivedWinshStateMotor == STATUS_MOTOR_SPEEDDOWN){ //green blink
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_GREEN);
   }
   else if (receivedWinshStateMotor == STATUS_MOTOR_RUNHIGH){ //green
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_GREEN);
   }
   else if (receivedWinshStateMotor == STATUS_MOTOR_STOPING){ //red
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_RED);
   }
   else if (receivedWinshStateMotor == STATUS_MOTOR_UNKNOWN){ //off
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_OFF);
   }
   else { //off
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_OFF);
   }

   //STATUS REMOTE
   if (lastStateRunningRemote == STATUS_REMOTE_EMERGENCY){
     myWS2812LEDs.setMode (WS2812_REMOTE_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningRemote == STATUS_REMOTE_IDLE){
     myWS2812LEDs.setMode (WS2812_REMOTE_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningRemote == STATUS_REMOTE_INIT){
     myWS2812LEDs.setMode (WS2812_REMOTE_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningRemote == STATUS_REMOTE_SHUTDOWN){
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_100, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_80, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_60, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_40, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_BAT_STATUS_LED_20, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_EMERGENCY_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_MOTOR_STATUS_LED , WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_MOTOR_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_PUSH_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_PUSH_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_PULL_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_PULL_STATUS_LED, WS2812_BLUE);
     myWS2812LEDs.setMode (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_WINSHTILT_STATUS_LED, WS2812_BLUE);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_STATUS_LED, WS2812_OFF);
   }

   //RFConnection
   if (transceivingProblemMaker == 0){
     myWS2812LEDs.setMode (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_GREEN);
   }
   else if (transceivingProblemMaker == 1){
     myWS2812LEDs.setMode (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_YELLOW);
   }
   else if (transceivingProblemMaker == 2){
     myWS2812LEDs.setMode (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_REMOTE_RFCONNCTION_STATUS_LED, WS2812_OFF);
   }


 }

 int checkStateToBe (int& pLastStateRunningRemote, int& pLastStateRunningEmergency, int& pLastStateRunningButton){
   int _StateToBeRemoteIs = STATUS_REMOTE_IDLE;
   int _StateToBeEmergencyIs = STATUS_EMERGENCY_NONE;
   int _StateToBeButtonIs = STATUS_BUTTON_NONE;

   buttonEmergency.checkSwitch();
   buttonPull.checkSwitch();
   buttonPush.checkSwitch();
   buttonSecondfunction.checkSwitch();
   buttonStart.checkSwitch();
   buttonStop.checkSwitch();


   //remember STATUS_SHUT_DOWN => STATE TO BE REMOTE
   if (pLastStateRunningRemote == STATUS_REMOTE_SHUTDOWN){
     _StateToBeRemoteIs = STATUS_REMOTE_SHUTDOWN;
   }

   //EMERGENCY CHECK => STATE TO BE EMERGENCY and => STATE TO BE REMOTE
   if (pLastStateRunningEmergency != STATUS_EMERGENCY_NONE){
     _StateToBeEmergencyIs = pLastStateRunningEmergency;
     _StateToBeRemoteIs = STATUS_REMOTE_EMERGENCY;
     if (buttonSecondfunction.isHeld ()){ //End of EMERGENCY
       _StateToBeEmergencyIs = STATUS_EMERGENCY_NONE;
       deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     }
   }
   if (buttonEmergency.isHeldLong ()){
     _StateToBeEmergencyIs = STATUS_EMERGENCY_EXTERN;
     _StateToBeRemoteIs = STATUS_REMOTE_EMERGENCY;
   }
   else if (buttonEmergency.isHeld ()){
     _StateToBeEmergencyIs = STATUS_EMERGENCY_LOCAL;
     _StateToBeRemoteIs = STATUS_REMOTE_EMERGENCY;
   }
   else if (buttonEmergency.isPressed ()){
     _StateToBeEmergencyIs = STATUS_EMERGENCY_STOP;
     _StateToBeRemoteIs = STATUS_REMOTE_EMERGENCY;
   }

   #ifdef IMU
     if (deadManTimeWillBe < millis ()){
       _StateToBeEmergencyIs = STATUS_EMERGENCY_DEADMAN;
       #ifdef DEBUG_IMU
         Serial.print ("DeadMan -----------------------------------------------------------------------------------!");
       #endif
     }
   #endif

   //check buttons including priority => STATE TO BE BUTTON
   if (pLastStateRunningButton == STATUS_BUTTON_PUSHLONG){ //remember function
     _StateToBeButtonIs = STATUS_BUTTON_PUSHLONG;
     if (buttonEmergency.isPressed () || buttonPull.isPressed() || buttonStop.isPressed () || buttonStart.isPressed () || buttonPush.pressed ()){
       _StateToBeButtonIs = STATUS_BUTTON_NONE;
     }
   }
   else if (buttonPush.isPressed ()){ //PUSH
     deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     if (buttonSecondfunction.isPressed ()){
       _StateToBeButtonIs = STATUS_BUTTON_PUSHLONG;
     }
     else {
       _StateToBeButtonIs = STATUS_BUTTON_PUSHSHORT;
     }
   }
   else if (buttonStop.isPressed ()){ //STOP -
     deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     if (buttonSecondfunction.isPressed ()){
       _StateToBeButtonIs = STATUS_BUTTON_MOTORSTOP;
     }
     else {
       if (pLastStateRunningButton == STATUS_BUTTON_MOTORSTOP){
         _StateToBeButtonIs = STATUS_BUTTON_MOTORSTOP;
       }
       else {
         _StateToBeButtonIs = STATUS_BUTTON_LOWTORQUE;
       }
     }
   }
   else if (buttonStart.isPressed ()){ //START +
     deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     if (buttonSecondfunction.isPressed ()){
       _StateToBeButtonIs = STATUS_BUTTON_MOTORSTART;
     }
     else {
       if (pLastStateRunningButton == STATUS_BUTTON_MOTORSTART){
         _StateToBeButtonIs = STATUS_BUTTON_MOTORSTART;
       }
       else {
         _StateToBeButtonIs = STATUS_BUTTON_HIGHTORQUE;
       }
     }
   }
   else if (buttonPull.isPressed ()){ //PULL
     deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     _StateToBeButtonIs = STATUS_BUTTON_PULL;
   }

   if (buttonSecondfunction.isHeldLong ()){ // SHUTDOWN
     deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
     timeAtShutoff = millis () + SWITCH_OFF_REMAINING_TIME;
     _StateToBeButtonIs = STATUS_BUTTON_SHUTDOWN;
     _StateToBeRemoteIs = STATUS_REMOTE_SHUTDOWN;
   }

   //RFConnection
   if (transceivingProblemMaker == 0){
     //OK
     if (_StateToBeRemoteIs != STATUS_REMOTE_SHUTDOWN){
       timeAtShutoff = millis () + TIME_AUTOMATIC_SWITCH_OFF;
     }
   }
   else if (transceivingProblemMaker == 1){
     //WORKING NOW AND THAN
   }
   else if (transceivingProblemMaker == 2){
     receivedRPM = 0;
     receivedWinshState = STATUS_WINCH_INIT;
     receivedWinshStateEmergency = STATUS_EMERGENCY_UNKNOWN;
     receivedWinshStateMotor = STATUS_MOTOR_UNKNOWN;
     receivedWinshStatePowertransmission = STATUS_POWERTRANSMISSION_UNKNOWN;
     receivedWinshStateTilt = STATUS_WINCH_TILT_UNKNOWN;
   }


   int change = 0;
   if (pLastStateRunningButton != _StateToBeButtonIs){
     pLastStateRunningButton = _StateToBeButtonIs;
     change = 1;
   }
   if (pLastStateRunningRemote != _StateToBeRemoteIs){
     pLastStateRunningRemote = _StateToBeRemoteIs;
     change = 2;
   }
   if (pLastStateRunningEmergency != _StateToBeEmergencyIs){
     pLastStateRunningEmergency = _StateToBeEmergencyIs;
     change = 3;
   }
   return change;
 }


 void setup(){
   //pins
   pinMode (MOSFET_POWER_PIN, OUTPUT);
   digitalWrite (MOSFET_POWER_PIN, HIGH);
   pinMode (BATTERIE_VOLTAGE_PIN, INPUT);

   //setup buttons
   buttonEmergency.begin();
   buttonPull.begin();
   buttonPush.begin();
   buttonSecondfunction.begin();
   buttonStart.begin();
   buttonStop.begin();
   buttonEmergency.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds
   buttonEmergency.holdTime(EMERGENCY_HOLD_TIME);//Default is 2 seconds
   buttonEmergency.longHoldTime(EMERGENCY_LONGHOLD_TIME);//Default is 5 seconds
   buttonPush.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds
   buttonSecondfunction.holdTime(EMERGENCY_RESET_HOLD_TIME);//Default is 2 seconds
   buttonSecondfunction.longHoldTime(SWITCH_OFF_LONGHOLD_TIME);//Default is 5 seconds
   buttonStart.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds
   buttonStop.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds
   buttonPull.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds

   #ifdef DEBUG_SERIAL
     //serial
     Serial.begin(SERIAL_SPEED);
   #endif

   //RF69
   myRF.inittt ();

   #ifdef IMU
     //MPU
     Wire.begin();
     // initialize device
     mySensor.setWire(&Wire);
     mySensor.beginAccel();

     #ifdef DEBUG_IMU
       mySensor.accelUpdate();
       Serial.println("print accel values");
       Serial.println("accelX: " + String(mySensor.accelX()));
       Serial.println("accelY: " + String(mySensor.accelY()));
       Serial.println("accelZ: " + String(mySensor.accelZ()));
       Serial.println("accelSqrt: " + String(mySensor.accelSqrt()));
     #endif
   #endif

   //WS2812
   myWS2812LEDs.setSpeed (WS2812LED_SPEED);
   red.r = 255;
   red.b = 0;
   red.g = 0;
   blue.r = 0;
   blue.b = int (float (2.55 * LIGHTFACTOR));
   blue.g = 0;
   green.r = 0;
   green.b = 0;
   green.g = int (float (2.55 * LIGHTFACTOR));
   yellow.r = int (float (2.55 * LIGHTFACTOR));
   yellow.b = 0;
   yellow.g = int (float (1.50 * LIGHTFACTOR));

   #ifdef DEBUG_SERIAL_MAIN
     //serial
     Serial.println ("Setup Ende");
     Serial.println (millis ());
   #endif
 }

 void loop(){
   #ifdef DEBUG_SERIAL_MAIN
     Serial.print ("Loop ");
     Serial.println (millis ());
     Serial.print (lastStateRunningRemote);
     Serial.print (" ");
     Serial.print (lastStateRunningEmergency);
     Serial.print (" ");
     Serial.println (lastStateRunningButton);
   #endif

   #ifdef IMU
     #ifdef DEBUG_IMU
       Serial.println (millis ());
     #endif
     float movemenG = 0;
     //delay (200);
     mySensor.accelUpdate();
     movemenG = mySensor.accelSqrt();
     if (movemenG < G_MOVEMENT_MIN || movemenG > G_MOVEMENT_MAX){
       if (lastStateRunningEmergency != STATUS_EMERGENCY_DEADMAN){
         deadManTimeWillBe = millis () + TIME_EMERGENCY_DEADMAN;
         #ifdef DEBUG_IMU
           Serial.print (movemenG);
           Serial.print (" Deadman Time Reset");
         #endif
       }
       #ifdef DEBUG_IMU
         Serial.print (movemenG);
         Serial.println (" Bewegt");
       #endif
     }
     #ifdef DEBUG_IMU
       //Serial.println ("");
       //Serial.println (millis ());
     #endif
   #endif


   //Voltmetertiming
   if (millis () > timingNextVoltmeter){
     timingNextVoltmeter = millis() + TIME_INTERVAL_FOR_VOLTMETER;
     batteryVoltage = map (analogRead(BATTERIE_VOLTAGE_PIN), BATTERIE_VOLTAGE_VALLUE_3_2V, BATTERIE_VOLTAGE_VALLUE_4_2V, 0, 100);
     batteryVoltage = constrain(batteryVoltage, 0, 150);
     #ifdef DEBUG_SERIAL_BAT
       //serial
       Serial.print ("BAT: ");
       Serial.println (analogRead(BATTERIE_VOLTAGE_PIN));
       Serial.print ("BAT%: ");
       Serial.println (batteryVoltage);
       Serial.println (millis ());
     #endif
   }

   //REMOTE_SHUTDOWN
   if (lastStateRunningRemote == STATUS_REMOTE_SHUTDOWN){
       timeRemainingToShutoff = (timeAtShutoff - millis()) / 1000;
       timeRemainingToShutoff = timeRemainingToShutoff & 0xFF;
       if (millis() > timeAtShutoff){
         executeShutdown ();
       }
   }

   //BUTTONS
   int checkbutton = 0;
   checkbutton = checkStateToBe (lastStateRunningRemote, lastStateRunningEmergency, lastStateRunningButton);

   //setLEDS
   checkLEDtoBe();
   #ifdef DEBUG_SERIAL_MAIN
     Serial.print ("LED ");
     Serial.println (millis());
   #endif
   if (checkbutton > 0){  //Quick update if butteon was pressed
     myWS2812LEDs.operate (true);
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("quick led update due to button");
     #endif
   }

   #ifdef DEBUG_SERIAL_MAIN
     Serial.println (millis());
   #endif

   //RF operate (send receive)
   int check = myRF.operateAndCheckAvaliable (lastStateRunningRemote, lastStateRunningEmergency, lastStateRunningButton, batteryVoltage, timeRemainingToShutoff);
   if (check == 1){
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("RF Operate 1");
     #endif
     transceivingProblemMaker = 0;
     receivedRPM = myRF.getRPM ();
     receivedWinshState = myRF.getWinchState ();
     receivedWinshStateEmergency = myRF.getWinchStateEmergency ();
     receivedWinshStateMotor = myRF.getWinchStateMotor ();
     receivedWinshStatePowertransmission = myRF.getWinchStatePowertransmission ();
     receivedWinshStateTilt = myRF.getWinchStateTilt ();
   }
   else if (check == 2){
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("RF Operate 2");
     #endif
     transceivingProblemMaker = 1;
   }
   else if (check == 3){
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("RF Operate 3");
     #endif
     transceivingProblemMaker = 2;
   }
   #ifdef DEBUG_SERIAL_MAIN
     Serial.println ("LEDS operate");
   #endif

   myWS2812LEDs.operate ();
   #ifdef DEBUG_SERIAL_MAIN
     Serial.println ("END LOOP");
   #endif
 }
