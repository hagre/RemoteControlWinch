

#ifndef RFTRANSCEIVER_h
#define RFTRANSCEIVER_h

#include "definitions.h"

#include <Arduino.h>

#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>

class RFTransceiver {

public:
  RFTransceiver (RH_RF69& myDriver, RHReliableDatagram& myManager);
  int inittt (void);
  void setPriority (bool priority);
  void setTxPower (int txPower);
  int operateAndCheckAvaliable (int input1, int input2, int input3, int input4, int input5, int input6 = 0);
  int setMessageRemote (int remoteState, int remoteStateEmergency, int remoteStateButton, int batteryVoltage, int timeToShutdown);
  void setMessageWinsh (int winchState, int winchStateEmergency, int winchStateMotor, int winchStatePowertransmission, int winchStateTilt, int RPM);
  int sendNowMessageRemote (void);
  int receiveNowMessageOnWinsh (void);
  int getRemoteState (void);
  int getRemoteStateEmergency (void);
  int getRemoteStateButton (void);
  int getBattery (void);
  int getTimeToShutdown (void);
  int getWinchState (void);
  int getWinchStateEmergency (void);
  int getWinchStateMotor (void);
  int getWinchStatePowertransmission (void);
  int getWinchStateTilt (void);
  int getRPM (void);


private:
  RH_RF69& _pMyDriver;
  RHReliableDatagram& _pMyManager;

  //void setFrequency (float centre);

  uint8_t _receive_buf [4] = {0,0,0,0};
  uint8_t _transmit_buf [4] = {0,0,0,0};
  int _thisRFAddress = -1;
  int _fromRFAddress = -1;
  int _txPower = -1;
  bool _remote = true;
  int _remoteState = -1;
  int _remoteStateEmergency = -1;
  int _remoteStateButton = -1;
  int _winchState = -1;
  int _winchStateEmergency = -1;
  int _winchStateMotor = -1;
  int _winchStateTilt = -1;
  int _winchStatePowertransmission = -1;
  int _battery = -1;
  int _timeToShutdown = -1;
  int _RPM = -1;
  int _rFconnectionStatus = 0;
  unsigned long _timeIntervallSend = -1;
  unsigned long _timeSinceLastACK = -1;
  unsigned long _timeSinceLastSend = -1;
};

#endif
