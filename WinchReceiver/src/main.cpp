/**
 * RECEIVER ON WICH FOR AGRICULTURAL WICH by hagre 2018
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
 // REMOTE CONTROL RECEIVER mounted on the agricultural winch, made with an Arduino Nano (5v).
 // To use the watchdog timer reset function you will need to flash the optiboot bootloader and
 // use the nano like an uno.
 // Status indications are done by WS2812 LEDs. Including differnet emergency levels and a stabile and relaible
 // RF connection including aknolegement of the receiver witch a 433mhz RF69 module. Some relays
 // to switch the 12v supply line for the pull and push valve. And a wtachdogtimer to reset
 // the system in case of an failure.
 //
 //  --------------------------------------------------------------------
 //  You will need the "LightWS2812" library.                           !
 //  It can be found under "ID159" in the PlatformIO library manager.   !
 //  https://github.com/cpldcpu/light_ws2812                            !
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
 #include <avr/wdt.h>

 //#define WINSH
 #include <definitions.h>
 //#define DEBUG_SERIAL
 //#define DEBUG_SERIAL_MAIN
 //#define DEBUG_SERIAL_TILT
 //#define DEBUG_SERIAL_WDT
 #define LIGHTFACTOR 50 //100%
 #define TIME_FOR_SERIAL_TO_RECEIVE_TILTSTATUS 1000

 #include <RHReliableDatagram.h>
 #include <RH_RF69.h>
 #include <SPI.h>
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
 //PMButton buttonEmergency (EMERGENCY_BUTTON_PIN);
 PMButton buttonPull (PULL_BUTTON_PIN);
 PMButton buttonPush (PUSH_BUTTON_PIN);
 //PMButton buttonSecondfunction (SECONDFUNCTION_BUTTON_PIN);

 int lastStateRunningWinch = STATUS_WINCH_INIT;
 int lastStateRunningEmergency = STATUS_EMERGENCY_NONE;
 int lastStateRunningMotor = STATUS_MOTOR_STOP;
 int lastStateRunningPowertransmission = STATUS_POWERTRANSMISSION_STOP;
 int lastStateRunningTilt = STATUS_WINCH_TILT_INIT;
 int CANtransceivingProblemMaker = 3; // 2;//3;

 int transceivingProblemMaker = -1;
 int receivedRemoteState = 0;
 int receivedRemoteStateEmergency = STATUS_EMERGENCY_UNKNOWN;
 int receivedRemoteStateButton = STATUS_BUTTON_NONE;
 int receivedRemoteBatterie = 160;
 int receivedRemoteTimeToShutdown = 0;

 int RPM = 0;

 unsigned long lastTimeSerialReceived = 0;

 void DoPush (){
   digitalWrite(BRAKE_AND_PUSH_RELAY_PIN, LOW);
   digitalWrite(PULL_RELAY_PIN, HIGH);
   digitalWrite(SAFETY_RELAY_PIN, LOW);
 }

 void DoPull (){
   digitalWrite(BRAKE_AND_PUSH_RELAY_PIN, HIGH);
   digitalWrite(PULL_RELAY_PIN, LOW);
   digitalWrite(SAFETY_RELAY_PIN, LOW);
 }

 void NoPushPull (){
   digitalWrite(BRAKE_AND_PUSH_RELAY_PIN, HIGH);
   digitalWrite(PULL_RELAY_PIN, HIGH);
   digitalWrite(SAFETY_RELAY_PIN, HIGH);
 }

 void checkMotor (){
   //Testsetting, no canbus used
   lastStateRunningMotor = STATUS_MOTOR_RUNHIGH;
 }

 void checkPowertransmission (){
   //Testsetting, no canbus/Sensor used
   lastStateRunningPowertransmission = STATUS_POWERTRANSMISSION_RUN;
 }

 void checkRPM (){
   //Testsetting, no canbus/Sensor used
   RPM = 400;
 }

 void checkTilt (){
   //Testsetting, no canbus/Sensor used
   //lastStateRunningTilt = 0;
   if (Serial.available()){
     switch (Serial.read()){
       case 'I': lastStateRunningTilt = STATUS_WINCH_TILT_ON_OK;
            lastTimeSerialReceived = millis ();
            #ifdef DEBUG_SERIAL_TILT
              Serial.println (" T_ON ");
            #endif
            break;
       case 'E': lastStateRunningTilt = STATUS_WINCH_TILT_EMERGENCY_CROSS;
            lastTimeSerialReceived = millis ();
            #ifdef DEBUG_SERIAL_TILT
              Serial.println (" T_EMERG ");
            #endif
            break;
       case 'W': lastStateRunningTilt = STATUS_WINCH_TILT_ALARM_CROSS;
            lastTimeSerialReceived = millis ();
            #ifdef DEBUG_SERIAL_TILT
              Serial.println (" T_WARN ");
            #endif
            break;
       case 'O': lastStateRunningTilt = STATUS_WINCH_TILT_OFF;
            lastTimeSerialReceived = millis ();
            #ifdef DEBUG_SERIAL_TILT
              Serial.println (" T_OFF ");
            #endif
            break;
       case 'X': lastStateRunningTilt = STATUS_WINCH_TILT_UNKNOWN;
            lastTimeSerialReceived = millis ();
            #ifdef DEBUG_SERIAL_TILT
              Serial.println (" T_? ");
            #endif
            break;
     }
   }
   if (millis () - lastTimeSerialReceived > TIME_FOR_SERIAL_TO_RECEIVE_TILTSTATUS){
     lastStateRunningTilt = STATUS_WINCH_TILT_UNKNOWN;
   }
 }

 int checkInputsAndDoRelays (){
   buttonPull.checkSwitch();
   buttonPush.checkSwitch();

   int _StateToBeWinchIs = STATUS_WINCH_IDLE;
   int _StateToBeEmergencyIs = STATUS_EMERGENCY_NONE;

   //EMERGENCY CHECK
   _StateToBeEmergencyIs = receivedRemoteStateEmergency;
   if (receivedRemoteStateEmergency == STATUS_EMERGENCY_STOP){
     //StopMotor();
     //StopPowerTransmission ();
     NoPushPull ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("STOPPPPP");
     #endif
   }

   //Motor

   //PowerTransmission

   //PUSH
   else if (receivedRemoteStateButton == STATUS_BUTTON_PUSHLONG || receivedRemoteStateButton == STATUS_BUTTON_PUSHSHORT ){
     if (transceivingProblemMaker == 0){
       _StateToBeWinchIs = STATUS_WINCH_PUSH;
       DoPush ();
       #ifdef DEBUG_SERIAL_MAIN
         Serial.println ("PUSH1");
       #endif
     }
   }
   else if (buttonPush.isPressed ()){
     _StateToBeWinchIs = STATUS_WINCH_PUSH;
     DoPush ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("PUSH2");
     #endif
   }
   //PULL
   else if (receivedRemoteStateButton == STATUS_BUTTON_PULL && lastStateRunningTilt != STATUS_WINCH_TILT_EMERGENCY_CROSS && lastStateRunningTilt != STATUS_WINCH_TILT_EMERGENCY_LENGTH && transceivingProblemMaker == 0){
     _StateToBeWinchIs = STATUS_WINCH_PULL;
     DoPull ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("PULL");
     #endif
   }
   else if (buttonPull.isPressed () && lastStateRunningTilt != STATUS_WINCH_TILT_EMERGENCY_CROSS && lastStateRunningTilt != STATUS_WINCH_TILT_EMERGENCY_LENGTH){
     _StateToBeWinchIs = STATUS_WINCH_PULL;
     DoPull ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("PULL2");
     #endif
   }
   else {
     _StateToBeWinchIs = STATUS_WINCH_IDLE;
     NoPushPull ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.println ("NOTHING");
     #endif
   }

   //RFConnection
   if (transceivingProblemMaker == 0){
     //OK
   }
   else if (transceivingProblemMaker == 1){
     //WORKING NOW AND THAN
   }
   else if (transceivingProblemMaker == 2){
     receivedRemoteState = STATUS_REMOTE_UNKNOWN;
     receivedRemoteStateButton = STATUS_BUTTON_NONE;
     receivedRemoteStateEmergency = STATUS_EMERGENCY_UNKNOWN;
     //receivedRemoteTimeToShutdown = ?;
     receivedRemoteBatterie = 160;
   }

   int change = 0;
   if (lastStateRunningWinch != _StateToBeWinchIs){
     lastStateRunningWinch = _StateToBeWinchIs;
     change = 2;
   }
   if (lastStateRunningEmergency != _StateToBeEmergencyIs){
     lastStateRunningEmergency = _StateToBeEmergencyIs;
     change = 3;
   }
   return change;
 }

 void doLedStatusBar (cRGB colorForBar, int indication){
   for (int i = 0; i < 60;  i++){
     myWS2812LEDs.setMode (WS2812_WINSH_STATUS_BAR_FRIST_LED_OF_60 + i, indication);
     myWS2812LEDs.setColor (WS2812_WINSH_STATUS_BAR_FRIST_LED_OF_60 + i, colorForBar);
   }
 }

 void doLEDs (){
   //Statusindication Leds
   //StatusBar60LEDs
   if (lastStateRunningEmergency == STATUS_EMERGENCY_DEADMAN || lastStateRunningEmergency == STATUS_EMERGENCY_EXTERN || lastStateRunningEmergency == STATUS_EMERGENCY_LOCAL ){ //red blink
     doLedStatusBar (WS2812_RED, WS2812_BLINK);
   }
   else if (lastStateRunningEmergency == STATUS_EMERGENCY_STOP){ //red
     doLedStatusBar (WS2812_RED, WS2812_ON);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_LENGTH){ //yellow blink
     doLedStatusBar (WS2812_RED, WS2812_ON);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_LENGTH){ //yellow blink
     doLedStatusBar (WS2812_YELLOW, WS2812_ON);
   }
   else if (lastStateRunningWinch == STATUS_WINCH_PULL){
     doLedStatusBar (WS2812_BLUE, WS2812_BLINK);
   }
   else if (lastStateRunningWinch == STATUS_WINCH_PUSH){
     doLedStatusBar (WS2812_BLUE, WS2812_ON);
   }
   else if (transceivingProblemMaker != 0 ){
    doLedStatusBar (WS2812_YELLOW, WS2812_BLINK);
   }
   else if (transceivingProblemMaker == 0 && lastStateRunningWinch == STATUS_WINCH_IDLE && (lastStateRunningPowertransmission != STATUS_POWERTRANSMISSION_RUN || lastStateRunningMotor != STATUS_MOTOR_RUNHIGH || lastStateRunningMotor != STATUS_MOTOR_RUNLOW)){
    doLedStatusBar (WS2812_GREEN, WS2812_BLINK);
   }
   else if (transceivingProblemMaker == 0 && lastStateRunningWinch == STATUS_WINCH_IDLE && lastStateRunningPowertransmission == STATUS_POWERTRANSMISSION_RUN && lastStateRunningMotor == STATUS_MOTOR_RUNHIGH && lastStateRunningMotor == STATUS_MOTOR_RUNLOW){
    doLedStatusBar (WS2812_GREEN, WS2812_ON);
   }
   else {
    doLedStatusBar (WS2812_GREEN, WS2812_OFF);
   }

   //Emergency
   if (lastStateRunningEmergency == STATUS_EMERGENCY_DEADMAN || lastStateRunningEmergency == STATUS_EMERGENCY_EXTERN || lastStateRunningEmergency == STATUS_EMERGENCY_LOCAL ){ //red blink
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningEmergency == STATUS_EMERGENCY_STOP){ //red
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_LENGTH){ //yellow blink
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_LENGTH){ //yellow blink
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningEmergency == STATUS_EMERGENCY_UNKNOWN){ //yellow blink
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_OFF);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_STATUS_LED, WS2812_OFF);
   }

   //Emergency extern to call
   if (lastStateRunningEmergency == STATUS_EMERGENCY_EXTERN){ //red blink
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_EXTERN_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_EMERGENCY_EXTERN_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_EMERGENCY_EXTERN_LED, WS2812_OFF);
   }

   //battery
   if (receivedRemoteBatterie >= 150){
       myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_OFF);
   }
   else if (receivedRemoteBatterie >= 50){
     myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_GREEN);
   }
   else if (receivedRemoteBatterie >= 30 && receivedRemoteBatterie < 50){
     myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_YELLOW);
   }
   else if (receivedRemoteBatterie >= 10 && receivedRemoteBatterie < 30){
     myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_RED);
   }
   else if (receivedRemoteBatterie < 10){
     myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_BAT_STATUS_LED_SOLO, WS2812_OFF);
   }

   //PULL
   if (lastStateRunningWinch == STATUS_WINCH_PULL){ //green
     myWS2812LEDs.setMode (WS2812_WINSH_PULL_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_PULL_STATUS_LED, WS2812_GREEN);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_PULL_STATUS_LED, WS2812_OFF);
   }

   //PUSH
   if (lastStateRunningWinch == STATUS_WINCH_PUSH){ //green
     myWS2812LEDs.setMode (WS2812_WINSH_PUSH_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_PUSH_STATUS_LED, WS2812_GREEN);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_PUSH_STATUS_LED, WS2812_OFF);
   }

   //powertransmission
   if (lastStateRunningPowertransmission == STATUS_POWERTRANSMISSION_STOP){ //red
     myWS2812LEDs.setMode (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningPowertransmission == STATUS_POWERTRANSMISSION_STARTING){ //yellow blink
     myWS2812LEDs.setMode (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningPowertransmission == STATUS_POWERTRANSMISSION_RUN){ //green
     myWS2812LEDs.setMode (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningPowertransmission == STATUS_POWERTRANSMISSION_STOPING){ //red
     myWS2812LEDs.setMode (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_POWERTRANSMISSION_STATUS_LED, WS2812_OFF);
   }

   //Motor
   if (lastStateRunningMotor == STATUS_MOTOR_STOP){ //red
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningMotor == STATUS_MOTOR_STARTING){ //yellow blink
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningMotor == STATUS_MOTOR_RUNLOW){ //blue
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_BLUE);
   }
   else if (lastStateRunningMotor == STATUS_MOTOR_SPEEDUP || lastStateRunningMotor == STATUS_MOTOR_SPEEDDOWN){ //green blink
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningMotor == STATUS_MOTOR_RUNHIGH){ //green
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningMotor == STATUS_MOTOR_STOPING){ //red
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_MOTOR_STATUS_LED, WS2812_OFF);
   }

   //STATUS WINSH
   if (lastStateRunningWinch == STATUS_WINCH_IDLE){
     myWS2812LEDs.setMode (WS2812_WINSH_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningWinch == STATUS_WINCH_PULL || lastStateRunningWinch == STATUS_WINCH_PUSH){
     myWS2812LEDs.setMode (WS2812_WINSH_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_STATUS_LED, WS2812_BLUE);
   }
   else if (lastStateRunningWinch == STATUS_WINCH_INIT){
     myWS2812LEDs.setMode (WS2812_WINSH_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_STATUS_LED, WS2812_YELLOW);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_STATUS_LED, WS2812_OFF);
   }

   //RFConnection
   if (transceivingProblemMaker == 0){
     myWS2812LEDs.setMode (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_GREEN);
   }
   else if (transceivingProblemMaker == 1){
     myWS2812LEDs.setMode (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_YELLOW);
   }
   else if (transceivingProblemMaker == 2){
     myWS2812LEDs.setMode (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_RED);
   }
   else if (transceivingProblemMaker == 2){
     myWS2812LEDs.setMode (WS2812_WINSH_RFCONNCTION_STATUS_LED, WS2812_OFF);
   }

   //CAN Connection
   if (CANtransceivingProblemMaker == 0){
     myWS2812LEDs.setMode (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_GREEN);
   }
   else if (CANtransceivingProblemMaker == 1){
     myWS2812LEDs.setMode (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_YELLOW);
   }
   else if (CANtransceivingProblemMaker == 2){
     myWS2812LEDs.setMode (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_CANCONNCTION_STATUS_LED, WS2812_OFF);
   }

   //Tilt
   if (lastStateRunningTilt == STATUS_WINCH_TILT_INIT){
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_TILT_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_OFF){ //
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_TILT_STATUS_LED, WS2812_YELLOW);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_ON_OK){
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_TILT_STATUS_LED, WS2812_GREEN);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_ALARM_LENGTH){
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_BLINK);
     myWS2812LEDs.setColor (WS2812_WINSH_TILT_STATUS_LED, WS2812_RED);
   }
   else if (lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_CROSS || lastStateRunningTilt == STATUS_WINCH_TILT_EMERGENCY_LENGTH){
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_ON);
     myWS2812LEDs.setColor (WS2812_WINSH_TILT_STATUS_LED, WS2812_RED);
   }
   else {
     myWS2812LEDs.setMode (WS2812_WINSH_TILT_STATUS_LED, WS2812_OFF);
   }
 }

 void setup() {
   wdt_reset();
   wdt_disable();
   pinMode (BRAKE_AND_PUSH_RELAY_PIN, OUTPUT);
   pinMode (PULL_RELAY_PIN, OUTPUT);
   pinMode (SAFETY_RELAY_PIN, OUTPUT);

   digitalWrite(BRAKE_AND_PUSH_RELAY_PIN, HIGH);
   digitalWrite(PULL_RELAY_PIN, HIGH);
   digitalWrite(SAFETY_RELAY_PIN, HIGH);

   //setup buttons
   buttonPull.begin();
   buttonPush.begin();
   buttonPush.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds
   buttonPull.debounce(DEBOUNCE_TIME);//Default is 10 milliseconds

   Serial.begin(SERIAL_SPEED);


   myRF.inittt ();

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

   wdt_enable(WDTO_4S);

   #ifdef DEBUG_SERIAL_MAIN
     Serial.println ("Setup Ende");
     Serial.println (millis ());
   #endif

 }

 void loop() {

   wdt_reset();
   #ifdef DEBUG_SERIAL_WDT
     if (millis () > 15000){
       Serial.println ("WDT");
       Serial.println (millis ());
       while(true);
     }
   #endif

   checkMotor ();
   checkPowertransmission ();
   checkRPM ();
   checkTilt ();

   int check = myRF.operateAndCheckAvaliable (lastStateRunningWinch, lastStateRunningEmergency, lastStateRunningMotor, lastStateRunningPowertransmission, lastStateRunningTilt, RPM);
   if (check == 1){
     transceivingProblemMaker = 0;
     receivedRemoteState = myRF.getRemoteState ();
     receivedRemoteStateEmergency = myRF.getRemoteStateEmergency ();
     receivedRemoteStateButton = myRF.getRemoteStateButton ();
     receivedRemoteTimeToShutdown = myRF.getTimeToShutdown ();
     receivedRemoteBatterie = myRF.getBattery ();
     #ifdef DEBUG_SERIAL_MAIN
       Serial.print ("operate positiv ");
       Serial.println (receivedRemoteState);
       Serial.println (millis ());
       Serial.print ("receivedRemoteState ");
       Serial.println (receivedRemoteState);
       Serial.print ("receivedRemoteStateEmergency ");
       Serial.println (receivedRemoteStateEmergency);
       Serial.print ("receivedRemoteStateButton ");
       Serial.println (receivedRemoteStateButton);
       Serial.print ("receivedRemoteTimeToShutdown ");
       Serial.println (receivedRemoteTimeToShutdown);
       Serial.print ("receivedRemoteBatterie ");
       Serial.println (receivedRemoteBatterie);
     #endif
   }
   else if (check == 2){
     transceivingProblemMaker = 1;
   }
   else if (check == 3){
     transceivingProblemMaker = 2;
   }

   checkInputsAndDoRelays ();
   doLEDs ();
   myWS2812LEDs.operate ();

   #ifdef DEBUG_SERIAL_MAIN
     Serial.print ("check: ");
     Serial.println (check);
     Serial.println (millis ());
   #endif
 }
