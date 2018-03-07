#include <RFTransceiver.h>
#include "definitions.h"

//#define DEBUG_SERIAL_RF

RFTransceiver::RFTransceiver (RH_RF69& myDriver, RHReliableDatagram& myManager) : _pMyDriver (myDriver), _pMyManager (myManager){
  _thisRFAddress = RF_ADDRESS;
  _fromRFAddress = RF_ADDRESS_OPOSIT;
  _txPower = RFTXPOWER;
  _timeIntervallSend = RFTXINTERVALL;
  //inittt ();
  if (_thisRFAddress == REMOTE_ADDRESS){
    _remote = true;
  }
  else {
    _remote = false;
  }
}

int RFTransceiver::inittt (void){
  _pMyManager.init();
  setPriority (false);
  _pMyDriver.setFrequency(FREQUENCY_SET);
  _pMyDriver.setTxPower(_txPower, true);
  byte key [] = RFASEKEY;
  _pMyDriver.setEncryptionKey (key);
  uint8_t rfsyncword [] = RFSYNCWORD;
  _pMyDriver.setSyncWords (rfsyncword, RFSYNCWORDLEN);
  //bool setModemConfig(ModemConfigChoice index);
  return 0;
}

void RFTransceiver::setPriority (bool priority){
  if (priority == true){
    _pMyManager.setTimeout (RFPRIORITY_TIMEOUT);
    _pMyManager.setRetries (RFPRIORITY_RETREIS);
    _timeIntervallSend = RFTXINTERVALL/2;
  }
  else {
    _pMyManager.setTimeout (RFNORMAL_TIMEOUT);
    _pMyManager.setRetries (RFNORMAL_RETREIS);
    _timeIntervallSend = RFTXINTERVALL;
  }
}

int RFTransceiver::operateAndCheckAvaliable (int input1, int input2, int input3, int input4, int input5, int input6 = 0){
  #ifdef DEBUG_SERIAL_RF
    //serial
    //Serial.print ("RFoperate ");
    //Serial.println (millis ());
  #endif

  int _done = 0;
  if (_remote == true){
    if (setMessageRemote (input1, input2, input3, input4, input5)){ //is there something new to send?
      //Serial.print ("RF somethin new ");
      if (sendNowMessageRemote ()){
        _done = 1;
      }
    }
    else if ((millis () - _timeSinceLastSend) > _timeIntervallSend){ // Time elapsed .retransmit
      //Serial.print ("RFtime elepsed ");
      if (sendNowMessageRemote ()){
        _done = 1;
      }
    }
    if (_rFconnectionStatus >= RF_RETRY_ERROR){
      _done = 2;
    }
    if ((millis () - _timeSinceLastACK) > RF_TIMEOUT_ERROR){ //RF connection problem ?
      //Serial.print ("RF time out error ");
      _done = 3;
    }
  }
  else {
    setMessageWinsh (input1, input2, input3, input4, input5, input6);
    if (receiveNowMessageOnWinsh ()){ //is there something new to send?
      _done = 1;
    }
    if (_rFconnectionStatus >= RF_RETRY_ERROR){
      _done = 2;
    }
    if ((millis () - _timeSinceLastACK) > RF_TIMEOUT_ERROR){ //RF connection problem ?
      _done = 3;
    }
  }
 return _done;
}

void RFTransceiver::setTxPower (int txPower){
  _txPower = txPower;
}

int RFTransceiver::setMessageRemote (int remoteState, int remoteStateEmergency, int remoteStateButton, int batteryVoltage, int timeToShutdown){
  int _new = 0;
  _remote = true;
  remoteState = constrain(remoteState, 0, 7);
  remoteStateEmergency = constrain(remoteStateEmergency, 0, 7);
  remoteStateButton = constrain(remoteStateButton, 0, 7);
  batteryVoltage = constrain(batteryVoltage, 0, 150);
  timeToShutdown = constrain(timeToShutdown, 0, 250);
  if (_remoteState != remoteState){
    _remoteState = remoteState;
    _new = 1;
  }
  if (_remoteStateEmergency != remoteStateEmergency){
    _remoteStateEmergency = remoteStateEmergency;
    _new = 1;
  }
  if (_remoteStateButton != remoteStateButton){
    _remoteStateButton = remoteStateButton;
    _new = 1;
  }
  if (_battery != batteryVoltage){
    _battery = batteryVoltage;
    _new = 1;
  }
  if (_timeToShutdown != timeToShutdown){
    _timeToShutdown = timeToShutdown;
    _new = 1;
  }
  return _new;
}

void RFTransceiver::setMessageWinsh (int winchState, int winchStateEmergency, int winchStateMotor, int winchStatePowertransmission, int winchStateTilt, int RPM){
  _remote = false;
  _winchState = constrain (winchState, 0, 7);
  _winchStateEmergency = constrain (winchStateEmergency, 0, 7);
  _winchStateMotor = constrain (winchStateMotor, 0, 7);
  _winchStatePowertransmission = constrain(winchStatePowertransmission, 0, 7);
  _winchStateTilt = constrain(winchStateTilt, 0, 7);
  _RPM = constrain (RPM, 0, 150);
  _transmit_buf [0] = (_winchState << 4) + _winchStateEmergency;
  _transmit_buf [1] = (_winchStateMotor << 4) + _winchStatePowertransmission;
  _transmit_buf [2] = _RPM;
  _transmit_buf [3] = _winchStateTilt;
  #ifdef DEBUG_SERIAL_RF
    //serial
    Serial.println ("_transmit_buf WINSH ");
    Serial.print (_transmit_buf [0], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [0], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [0]);
    Serial.print (_transmit_buf [1], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [1], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [1]);
    Serial.print (_transmit_buf [2], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [2], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [2]);
    Serial.print (_transmit_buf [3], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [3], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [3]);
  #endif
}

int RFTransceiver::sendNowMessageRemote (void){
  int _done = 0;
  _transmit_buf [0] = (_remoteState << 4) + _remoteStateEmergency;
  _transmit_buf [1] = _remoteStateButton;
  _transmit_buf [2] = _battery;
  _transmit_buf [3] = _timeToShutdown;
  #ifdef DEBUG_SERIAL_RF
    //serial
    Serial.println ("_transmit_buf REMOTE TTTT");
    Serial.print (_transmit_buf [0], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [0], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [0]);
    Serial.print (_transmit_buf [1], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [1], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [1]);
    Serial.print (_transmit_buf [2], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [2], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [2]);
    Serial.print (_transmit_buf [3], BIN);
    Serial.print (" BIN, ");
    Serial.print (_transmit_buf [3], HEX);
    Serial.print (" HEX, ");
    Serial.println (_transmit_buf [3]);
  #endif
  // Send a message to myManager_server
  _timeSinceLastSend = millis();
  //int from = _fromRFAddress;
  if (_pMyManager.sendtoWait(_transmit_buf, 4, _fromRFAddress))  {
    // Now wait for a reply from the server
    uint8_t len = 4;
    uint8_t from;
    if (_pMyManager.recvfromAckTimeout(_receive_buf, &len, RF_RECEIVE_TIMEOUT, &from)){
      #ifdef DEBUG_SERIAL_RF
        //serial
        Serial.print ("ACK true ");
        //Serial.println (millis ());
      #endif
      if (from == _fromRFAddress){
        #ifdef DEBUG_SERIAL_RF
          //serial
          Serial.println ("FROM true ");
          //Serial.println (millis ());
        #endif
        _timeSinceLastACK = millis();
        _winchState = _receive_buf [0] >> 4;
        _winchStateEmergency = _receive_buf [0] & 0x0F;
        _winchStateMotor = _receive_buf [1] >> 4;
        _winchStatePowertransmission = _receive_buf [1] & 0x0F;
        _RPM = _receive_buf [2];
        _winchStateTilt = _receive_buf [3];
        _rFconnectionStatus = 0;
        #ifdef DEBUG_SERIAL_RF
          //serial
          Serial.println ("_receive_buf REMOTE RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
          Serial.print (_receive_buf [0], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [0], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [0]);
          Serial.print (_receive_buf [1], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [1], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [1]);
          Serial.print (_receive_buf [2], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [2], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [2]);
          Serial.print (_receive_buf [3], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [3], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [3]);
        #endif
        _done = 1;
      }
    }
    else {
      _rFconnectionStatus = _rFconnectionStatus + 1;
    }
  }
  else {
  }
  return _done;
}

int RFTransceiver::receiveNowMessageOnWinsh (void){
  int _done = 0;
  if (_pMyManager.available()){
    uint8_t len = 4;
    uint8_t from;
    if (_pMyManager.recvfromAckTimeout(_receive_buf, &len, RF_RECEIVE_TIMEOUT, &from)){
      if (from == _fromRFAddress){
        _done = 1;
        _remoteState = _receive_buf [0] >> 4;
        _remoteStateEmergency = _receive_buf [0] & 0x0F;
        _remoteStateButton = _receive_buf [1] & 0x0F;
        _battery = _receive_buf [2];
        _timeToShutdown = _receive_buf [3];
        #ifdef DEBUG_SERIAL_RF
          //serial
          Serial.println ("_receive_buf WINSH RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
          Serial.print (_receive_buf [0], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [0], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [0]);
          Serial.print (_receive_buf [1], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [1], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [1]);
          Serial.print (_receive_buf [2], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [2], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [2]);
          Serial.print (_receive_buf [3], BIN);
          Serial.print (" BIN, ");
          Serial.print (_receive_buf [3], HEX);
          Serial.print (" HEX, ");
          Serial.println (_receive_buf [3]);
          Serial.print ("_receive_buf WINSH BUTTON STATE");
          Serial.println (_remoteStateButton);
        #endif
        _timeSinceLastSend = millis();
        if (_pMyManager.sendtoWait(_transmit_buf, 4, _fromRFAddress)){
          _timeSinceLastACK = millis ();
          _rFconnectionStatus = 0;
        }
        else {
          _rFconnectionStatus = _rFconnectionStatus + 1;
        }
      }
    }
  }
  return _done;
}

int RFTransceiver::getRemoteState (void){
  return _remoteState;
}

int RFTransceiver::getRemoteStateEmergency (void){
  return _remoteStateEmergency;
}

int RFTransceiver::getRemoteStateButton (void){
  return _remoteStateButton;
}

int RFTransceiver::getWinchState (void){
  return _winchState;
}

int RFTransceiver::getWinchStateEmergency (void){
  return _winchStateEmergency;
}

int RFTransceiver::getWinchStateMotor (void){
  return _winchStateMotor;
}

int RFTransceiver::getWinchStatePowertransmission (void){
  return _winchStatePowertransmission;
}

int RFTransceiver::getWinchStateTilt (void){
  return _winchStateTilt;
}

int RFTransceiver::getBattery (void){
  return _battery;
}

int RFTransceiver::getTimeToShutdown (void){
  return _timeToShutdown;
}

int RFTransceiver::getRPM (void){
  return _RPM;
}
