#define REMOTE //WINSH
//#define WINSH

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//ADDRESS OF EQUIPMENT
#define REMOTE_ADDRESS 1
#define WINCH_ADDRESS 2

#ifdef REMOTE
 #define RF_ADDRESS REMOTE_ADDRESS
 #define RF_ADDRESS_OPOSIT WINCH_ADDRESS
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ///   RF69
 ///
 ///                 Arduino      RFM69W
 ///                 GND----------GND   (ground in)
 ///                 3V3----------3.3V  (3.3V in)
 /// interrupt 0 pin D2-----------DIO0  (interrupt request out)
 ///          SS pin D10----------NSS   (chip select in)
 ///         SCK pin D13----------SCK   (SPI clock in)
 ///        MOSI pin D11----------MOSI  (SPI Data in)
 ///        MISO pin D12----------MISO  (SPI Data out)
 #define RFTRANSCEIVER_NSS 10
 #define RFTRANSCEIVER_DIO 2
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ///  I2C         Arduino         I2C
 ///                 GND----------GND
 ///                 3V3----------3.3V  ? or 5v
 ///         SDA pin  A4----------SDA
 ///         SCL pin  A5----------SCL
 #define SDA_PIN A4
 #define SCL_PIN A5
 //MPU9250
 #define G_MOVEMENT_MIN 0.95
 #define G_MOVEMENT_MAX 1.05
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// LED             Arduino      WS2812 LED
 ///                 GND----------GND
 ///         external 5v----------5V
 ///         DATA pin D7----------DATA
 #define WS2812_DATA_PIN 7
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// BUTTONS        Arduino      BUTTONS
 ///                 GND----------GND
 ///                  A0----------EMERGENCY_BUTTON
 ///                  A1----------PULL_BUTTON
 ///                  A2----------PUSH_BUTTON
 ///                  A3----------SECONDFUNCTION_BUTTON
 ///                  D6----------START/PLUS
 ///                  D8----------STOP/MINUS
 #define EMERGENCY_BUTTON_PIN A0
 #define PULL_BUTTON_PIN A1
 #define PUSH_BUTTON_PIN A2
 #define SECONDFUNCTION_BUTTON_PIN A3
 #define START_PLUS_BUTTON_PIN 6
 #define STOP_MINUS_BUTTON_PIN 5
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// POWER REMOTE  Arduino      MOSFET_POWER
 ///                 GND----------GND
 ///                  D9----------MOSFET
 #define MOSFET_POWER_PIN 9
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// BATTERY      Arduino      BatterieVoltage
 ///                 GND----------GND
 ///                  A6----------Voltagedivider LiPo
 #define BATTERIE_VOLTAGE_PIN A6
 #define BATTERIE_VOLTAGE_VALLUE_3_2V 690
 #define BATTERIE_VOLTAGE_VALLUE_4_2V 910

#endif

#ifdef WINSH
 #define RF_ADDRESS WINCH_ADDRESS
 #define RF_ADDRESS_OPOSIT REMOTE_ADDRESS
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ///   RF69
 ///
 ///                 Arduino      RFM69W
 ///                 GND----------GND   (ground in)
 ///                 3V3----------3.3V  (3.3V in)
 /// interrupt 0 pin D2-----------DIO0  (interrupt request out)
 ///          SS pin D10----------NSS   (chip select in)
 ///         SCK pin D13----------SCK   (SPI clock in)
 ///        MOSI pin D11----------MOSI  (SPI Data in)
 ///        MISO pin D12----------MISO  (SPI Data out)
 ///    SPI RF69 and/or  CANBUS
 ///                  D9----------chip select CANBUS MODUL
 ///                  D2----------INTERUPT
 #define RFTRANSCEIVER_NSS 10
 #define RFTRANSCEIVER_DIO 2
 #define CS_CANBUS_PIN 9
 #define INTERUPT_CANBUS_PIN 3
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// LED             Arduino      WS2812 LED
 ///                 GND----------GND
 ///         external 5v----------5V
 ///         DATA pin D7----------DATA
 #define WS2812_DATA_PIN 7
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// BUTTONS        Arduino      BUTTONS
 ///                 GND----------GND
 ///                  A1----------PULL_BUTTON
 ///                  A2----------PUSH_BUTTON
 #define PULL_BUTTON_PIN A1
 #define PUSH_BUTTON_PIN A2
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// RELAY WINSH   Arduino      RELAYS
 ///                 GND----------GND
 ///                  D9----------BRAKE_AND_PUSH_RELAY
 ///                  D4----------PULL_RELAY_PIN
 #define BRAKE_AND_PUSH_RELAY_PIN 8
 #define PULL_RELAY_PIN 6
 #define SAFETY_RELAY_PIN 5
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 /// Tilt Interrupt Arduino      Tilt
 ///                 GND----------GND
 ///                  D3----------Tilt SIGNAL
 ///                  D4----------Tilt ACTIVE ESP8266
 #define TILT_INTERRUPT_PIN A3
 #define TILT_ACTIVE_PIN A4
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///   RF69
#define FREQUENCY_SET 444,0
#define RFTXPOWER 20 //17
#define RFTXINTERVALL 100//1000
#define RFASEKEY { 0x3a, 0xb4, 0x12, 0x04, 0x11, 0xaf, 0x63, 0x42, 0xff, 0x89, 0x92, 0x12, 0x13 , 0x14, 0x15 , 0x16 }
#define RFSYNCWORD { 0x3a, 0xb4 }
#define RFSYNCWORDLEN 2
#define RF_TIMEOUT_ERROR 2000
#define RF_RETRY_ERROR 1
#define RF_RECEIVE_TIMEOUT 100
#define RFPRIORITY_TIMEOUT 50
#define RFPRIORITY_RETREIS 5
#define RFNORMAL_TIMEOUT 100
#define RFNORMAL_RETREIS 2//2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// LED             Arduino      WS2812 LED
#define WS2812_NUMBER_OF_LEDS_REMOTE 19
#define WS2812_NUMBER_OF_LEDS_WINCH 73
#ifdef REMOTE
  #define WS2812_NUMBER_OF_LEDS WS2812_NUMBER_OF_LEDS_REMOTE
#endif
#ifdef WINSH
  #define WS2812_NUMBER_OF_LEDS WS2812_NUMBER_OF_LEDS_WINCH
#endif

#define WS2812_REMOTE_STATUS_LED 5
#define WS2812_REMOTE_EMERGENCY_STATUS_LED 13
#define WS2812_REMOTE_MOTOR_STATUS_LED 8
#define WS2812_REMOTE_POWERTRANSMISSION_STATUS_LED 9
#define WS2812_REMOTE_PUSH_STATUS_LED 11
#define WS2812_REMOTE_PULL_STATUS_LED 10
#define WS2812_REMOTE_WINSHTILT_STATUS_LED 12
#define WS2812_REMOTE_RFCONNCTION_STATUS_LED 7
#define WS2812_REMOTE_BAT_STATUS_LED_SOLO 6
#define WS2812_REMOTE_BAT_STATUS_LED_20 14
#define WS2812_REMOTE_BAT_STATUS_LED_40 15
#define WS2812_REMOTE_BAT_STATUS_LED_60 16
#define WS2812_REMOTE_BAT_STATUS_LED_80 17
#define WS2812_REMOTE_BAT_STATUS_LED_100 18

#define WS2812_REMOTE_EMERGENCY_BUTTON_LED 0
#define WS2812_REMOTE_PULL_BUTTON_LED 3
#define WS2812_REMOTE_PUSH_BUTTON_LED 1
#define WS2812_REMOTE_SECONDFUNCTION_BUTTON_LED 4
#define WS2812_REMOTE_START_STOP_BUTTON_LED 2

#define WS2812_WINSH_STATUS_LED 0
#define WS2812_WINSH_EMERGENCY_STATUS_LED 6
#define WS2812_WINSH_EMERGENCY_EXTERN_LED 5
#define WS2812_WINSH_MOTOR_STATUS_LED 3
#define WS2812_WINSH_POWERTRANSMISSION_STATUS_LED 4
#define WS2812_WINSH_TILT_STATUS_LED 7
#define WS2812_WINSH_RFCONNCTION_STATUS_LED 1
#define WS2812_WINSH_CANCONNCTION_STATUS_LED 2
#define WS2812_WINSH_PULL_STATUS_LED 9
#define WS2812_WINSH_PUSH_STATUS_LED 8
#define WS2812_WINSH_BAT_STATUS_LED_SOLO 10
#define WS2812_WINSH_STATUS_BAR_FRIST_LED_OF_60 13

#define WS2812LED_SPEED 600

#define WS2812_ON 1
#define WS2812_OFF 0
#define WS2812_BLINK 2
#define WS2812_BLINK_ON 3
#define WS2812_BLINK_OFF 4
#define WS2812_RED red
#define WS2812_BLUE blue
#define WS2812_GREEN green
#define WS2812_YELLOW yellow

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   TIMINGS / SPEEDS
#define SERIAL_SPEED 115200
#define DEBOUNCE_TIME 25
#define EMERGENCY_HOLD_TIME 5000
#define EMERGENCY_LONGHOLD_TIME 10000
#define EMERGENCY_RESET_HOLD_TIME 5000
#define SWITCH_OFF_LONGHOLD_TIME 10000
#define SWITCH_OFF_REMAINING_TIME 5000
#define TIME_INTERVAL_FOR_VOLTMETER 2000
#define TIME_EMERGENCY_DEADMAN 600000 //10min
#define TIME_AUTOMATIC_SWITCH_OFF 3600000 //1h

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   STATUS
#define STATUS_REMOTE_INIT 0 //Init OK, but no Radio and CANBus, .... connection
#define STATUS_REMOTE_IDLE 1
#define STATUS_REMOTE_EMERGENCY 2
#define STATUS_REMOTE_SHUTDOWN 3
#define STATUS_REMOTE_UNKNOWN 4

#define STATUS_WINCH_INIT 0
#define STATUS_WINCH_IDLE 1
#define STATUS_WINCH_PULL 2
#define STATUS_WINCH_PUSH 3
#define STATUS_WINCH_UNKNOWN 4

#define STATUS_WINCH_TILT_INIT 0
#define STATUS_WINCH_TILT_OFF 1
#define STATUS_WINCH_TILT_ON_OK 2
#define STATUS_WINCH_TILT_ALARM_CROSS 3
#define STATUS_WINCH_TILT_ALARM_LENGTH 4
#define STATUS_WINCH_TILT_EMERGENCY_CROSS 5
#define STATUS_WINCH_TILT_EMERGENCY_LENGTH 6
#define STATUS_WINCH_TILT_UNKNOWN 7

#define STATUS_EMERGENCY_NONE 0
#define STATUS_EMERGENCY_STOP 1
#define STATUS_EMERGENCY_LOCAL 2
#define STATUS_EMERGENCY_EXTERN 3
#define STATUS_EMERGENCY_DEADMAN 4
#define STATUS_EMERGENCY_UNKNOWN 5

#define STATUS_MOTOR_STOP 0
#define STATUS_MOTOR_STARTING 1
#define STATUS_MOTOR_RUNLOW 2
#define STATUS_MOTOR_SPEEDUP 3
#define STATUS_MOTOR_RUNHIGH 4
#define STATUS_MOTOR_SPEEDDOWN 5
#define STATUS_MOTOR_STOPING 6
#define STATUS_MOTOR_UNKNOWN 7

#define STATUS_POWERTRANSMISSION_STOP 0
#define STATUS_POWERTRANSMISSION_STARTING 1
#define STATUS_POWERTRANSMISSION_RUN 2
#define STATUS_POWERTRANSMISSION_STOPING 3
#define STATUS_POWERTRANSMISSION_UNKNOWN 4

#define STATUS_BUTTON_NONE 0
#define STATUS_BUTTON_PUSHSHORT 1
#define STATUS_BUTTON_PUSHLONG 2
#define STATUS_BUTTON_PULL 3
#define STATUS_BUTTON_MOTORSTART 4
#define STATUS_BUTTON_MOTORSTOP 5
#define STATUS_BUTTON_HIGHTORQUE 6
#define STATUS_BUTTON_LOWTORQUE 7
#define STATUS_BUTTON_POWERTRANSMISSIONSTART 8
#define STATUS_BUTTON_POWERTRANSMISSIONSTOP 9
#define STATUS_BUTTON_EMERGENCYSTOP 10
#define STATUS_BUTTON_EMERGENCYLOCAL 11
#define STATUS_BUTTON_EMERGENCYEXTERN 12
#define STATUS_BUTTON_EMERGENCYDEADMAN 13
#define STATUS_BUTTON_EMERGENCYRESET 14
#define STATUS_BUTTON_SHUTDOWN 15
