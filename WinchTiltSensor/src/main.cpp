/**
 * TILT SENSOR FOR AGRICULTURAL WICH by hagre 2018
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


// Thes code is based on the MPU9250 Basic Example Code by Kris Winer
// and was modified and extended by hagre 2018

// TiltSensor for an agricultural winch made with an Wemos d1 mini lite (ESP8266) and an
// MPU9250 sensor connected via I2C. This sensor is to prevent extensive movement (tilt)
// of an agricultural wich during pulling operation. If the #defined limits are reached a relay
// will disconect the power supply to the electromagnetic pull valve of the winch. There is also
// the posibility to overide this sensor/module.
//
// This is an optional equipment as a try to gain more safety during winch operation.
// It should be used with the remote control (transmitter and receiver) of this project.
//
//  --------------------------------------------------------------------
//  You will need the "SparkFun MPU-9250 9 DOF IMU Breakout" library.  !
//  It can be found under "ID944" in the PlatformIO library manager.   !
//  https://github.com/sparkfun/MPU-9250_Breakout                      !
//  --------------------------------------------------------------------


/* MPU9250 Basic Example Code
 by: Kris Winer
 date: April 1, 2014
 license: Beerware - Use this code however you'd like. If you
 find it useful you can buy me a beer some time.
 Modified by Brent Wilkins July 19, 2016
 */


#include <Arduino.h>

#include "quaternionFilters.h"
#include "MPU9250.h"

//#define SERIAL_DEBUG

#define RELAY_PIN 2
#define SWITCHSENSOR_PIN 16
#define STOP_SIGNAL_PIN 14
#define OK_SIGNAL_PIN 12

#define TOOGLEINTERVAL 500;

#define ROLL_CORRETION 0.0
#define PITCH_CORRETION 0.0

#define EMERGENCYSTOP_LIMIT_CROSS 20
#define EMERGENCYSTOP_LIMIT_LENGTH 25
#define WARNING_LIMIT_CROSS 10
#define WARNING_LIMIT_LENGTH 15

#define SENSOR_STATUS_INIT 0
#define SENSOR_STATUS_ON_OK 1
#define SENSOR_STATUS_WARNING 2
#define SENSOR_STATUS_EMERGENCYSTOP 3
#define SENSOR_STATUS_OFF 4
#define SENSOR_STATUS_UNKNOWN 5

int actualStatus = SENSOR_STATUS_UNKNOWN;
int OK_Status = 1;
unsigned long nextToogleTime = 0;

MPU9250 myIMU;

void SwitchWinchOff (void){
  digitalWrite (RELAY_PIN, HIGH); //OFF
}

void SwitchWinchOn (void){
  digitalWrite (RELAY_PIN, LOW); //ON
}

void statusOutput (){
  if (actualStatus == SENSOR_STATUS_ON_OK){
    #ifdef SERIAL_DEBUG
      Serial.print  ("OK ");
    #endif
    digitalWrite (STOP_SIGNAL_PIN, LOW);
    if (millis () > nextToogleTime){
      if (OK_Status == 1){
        digitalWrite (OK_SIGNAL_PIN, LOW);
        OK_Status = 0;
        #ifdef SERIAL_DEBUG
          Serial.print (" T0 ");
        #endif
      }
      else {
        digitalWrite (OK_SIGNAL_PIN, HIGH);
        OK_Status = 1;
        #ifdef SERIAL_DEBUG
          Serial.print (" T1 ");
        #endif
      }
      nextToogleTime = millis () + TOOGLEINTERVAL;
    }
  }
  else if (actualStatus == SENSOR_STATUS_EMERGENCYSTOP){
    #ifdef SERIAL_DEBUG
      Serial.print (" STOP ");
    #endif
    digitalWrite (OK_SIGNAL_PIN, LOW);
    digitalWrite (STOP_SIGNAL_PIN, HIGH);
  }
  else if (actualStatus == SENSOR_STATUS_WARNING){
    #ifdef SERIAL_DEBUG
      Serial.print (" WARNING ");
    #endif
    digitalWrite (STOP_SIGNAL_PIN, HIGH);
    if (millis () > nextToogleTime){
      if (OK_Status == 1){
        digitalWrite (OK_SIGNAL_PIN, LOW);
        OK_Status = 0;
        #ifdef SERIAL_DEBUG
          Serial.print (" T0 ");
        #endif
      }
      else {
        digitalWrite (OK_SIGNAL_PIN, HIGH);
        OK_Status = 1;
        #ifdef SERIAL_DEBUG
          Serial.print (" T1 ");
        #endif
      }
      nextToogleTime = millis () + TOOGLEINTERVAL;
    }
  }
  else {
    #ifdef SERIAL_DEBUG
      Serial.print (" ELSE OFF ");
    #endif
    digitalWrite (OK_SIGNAL_PIN, LOW);
    digitalWrite (STOP_SIGNAL_PIN, LOW);
  }
}

int checkExternalOverideButton (){
  if (digitalRead (SWITCHSENSOR_PIN) == HIGH){ //debub ---------------------debug
    actualStatus = SENSOR_STATUS_OFF;
    #ifdef SERIAL_DEBUG
      Serial.println (" OVERIDE ");
    #endif
  }
  else {

  }
}

void setup(){
  pinMode (RELAY_PIN, OUTPUT);
  pinMode (STOP_SIGNAL_PIN, OUTPUT);
  digitalWrite (STOP_SIGNAL_PIN, LOW);
  pinMode (OK_SIGNAL_PIN, OUTPUT);
  digitalWrite (OK_SIGNAL_PIN, LOW);
  pinMode (SWITCHSENSOR_PIN, INPUT_PULLUP);
      digitalWrite (OK_SIGNAL_PIN, HIGH); // debub------------only-----------debug
  SwitchWinchOff ();
  actualStatus = SENSOR_STATUS_INIT;
  Wire.begin();
  #ifdef SERIAL_DEBUG
    Serial.begin(115200);
  #endif
  // Start by performing self test and reporting values
  myIMU.MPU9250SelfTest(myIMU.SelfTest);
  #ifdef SERIAL_DEBUG
    Serial.print("x-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[0],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[1],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: acceleration trim within : ");
    Serial.print(myIMU.SelfTest[2],1); Serial.println("% of factory value");
    Serial.print("x-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[3],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[4],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: gyration trim within : ");
    Serial.print(myIMU.SelfTest[5],1); Serial.println("% of factory value");
  #endif
  // Calibrate gyro and accelerometers, load biases in bias registers
  myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
  myIMU.initMPU9250();
  // Initialize device for active mode read of acclerometer, gyroscope, and
  // temperature
  #ifdef SERIAL_DEBUG
    Serial.println("MPU9250 initialized for active data mode....");
  #endif
  // Get magnetometer calibration from AK8963 ROM
  myIMU.initAK8963(myIMU.magCalibration);
  // Initialize device for active mode read of magnetometer
  #ifdef SERIAL_DEBUG
    Serial.println("AK8963 initialized for active data mode....");
    //  Serial.println("Calibration values: ");
    Serial.print("X-Axis sensitivity adjustment value ");
    Serial.println(myIMU.magCalibration[0], 2);
    Serial.print("Y-Axis sensitivity adjustment value ");
    Serial.println(myIMU.magCalibration[1], 2);
    Serial.print("Z-Axis sensitivity adjustment value ");
    Serial.println(myIMU.magCalibration[2], 2);
  #endif
}

void loop(){
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();
    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0]*myIMU.aRes; // - accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes; // - accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2]*myIMU.aRes; // - accelBias[2];
    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
    myIMU.getGres();
    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0]*myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1]*myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2]*myIMU.gRes;
    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
    myIMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    myIMU.magbias[0] = +470.;
    // User environmental x-axis correction in milliGauss TODO axis??
    myIMU.magbias[1] = +120.;
    // User environmental x-axis correction in milliGauss
    myIMU.magbias[2] = +125.;
    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0] -
               myIMU.magbias[0];
    myIMU.my = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1] -
               myIMU.magbias[1];
    myIMU.mz = (float)myIMU.magCount[2]*myIMU.mRes*myIMU.magCalibration[2] -
               myIMU.magbias[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  // Must be called before updating quaternions!
  myIMU.updateTime();
  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx*DEG_TO_RAD,
                         myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);
  // Serial print and/or display at 0.5 s rate independent of data rates
  myIMU.delt_t = millis() - myIMU.count;
  myIMU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() *
                *(getQ()+2)));
  myIMU.roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2) *
                *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1) * *(getQ()+1)
                - *(getQ()+2) * *(getQ()+2) + *(getQ()+3) * *(getQ()+3));
  myIMU.pitch *= RAD_TO_DEG;
  myIMU.roll  *= RAD_TO_DEG;

  float correctedRoll = myIMU.roll + ROLL_CORRETION;
  float correctedPitch = myIMU.pitch + PITCH_CORRETION;

  if (correctedPitch < - EMERGENCYSTOP_LIMIT_LENGTH || correctedPitch > EMERGENCYSTOP_LIMIT_LENGTH){
    SwitchWinchOff ();
    actualStatus = SENSOR_STATUS_EMERGENCYSTOP;
    #ifdef SERIAL_DEBUG
      Serial.println("PITCH STOP");
    #endif
  }
  else if (correctedRoll < - EMERGENCYSTOP_LIMIT_CROSS || correctedRoll > EMERGENCYSTOP_LIMIT_CROSS){
    SwitchWinchOff ();
    actualStatus = SENSOR_STATUS_EMERGENCYSTOP;
    #ifdef SERIAL_DEBUG
      Serial.println("ROLL STOP");
    #endif
  }
  else if (correctedPitch < - WARNING_LIMIT_LENGTH || correctedPitch > WARNING_LIMIT_LENGTH ){
    actualStatus = SENSOR_STATUS_WARNING;
    SwitchWinchOn ();
    #ifdef SERIAL_DEBUG
      Serial.println("PITCH WARNING");
    #endif
  }
  else if (correctedRoll < - WARNING_LIMIT_CROSS || correctedRoll > WARNING_LIMIT_CROSS){
    actualStatus = SENSOR_STATUS_WARNING;
    SwitchWinchOn ();
    #ifdef SERIAL_DEBUG
      Serial.println("ROLL WARNING");
    #endif
  }
  else {
    actualStatus = SENSOR_STATUS_ON_OK;
    SwitchWinchOn ();
  }

  checkExternalOverideButton ();
  statusOutput ();


  #ifdef SERIAL_DEBUG
    // update LCD once per half-second independent of read rate
    if (myIMU.delt_t > 500) {
      Serial.print("Pitch, Roll: ");
      Serial.print(myIMU.pitch, 2);
      Serial.print(", ");
      Serial.println(myIMU.roll, 2);
      Serial.print("rate = ");
      Serial.print((float)myIMU.sumCount/myIMU.sum, 2);
      Serial.println(" Hz");
      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } // if (myIMU.delt_t > 500)
  #endif
}
