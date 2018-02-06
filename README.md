# Remote-Control-Winch
Remote Control of an agricultural Winch


This project consists of 3 individual parts:

+ Remote (mobile transmitter)
+ Receiver (mounted on the winch)
+ an optional tilt sensor

I used Atom with Atom editor to write the code. Each folder should include all relevant information.



License

The code is published under the GPL license because I believe in open development. Do not use this code in products that are closed source or are crippled by a patent. See licence file for details.



Safety and Warranty

An agricultural winch can be very dangerous, please be extremely cautious during your work in the woods.
!!! There is absolutely NO WARRANTY for this remote control !!! Test the software and hardware extensively. Do not rely on it.
I had just some freezes on my remote after 10 min up to 2 hours. I need to disconnect the battery to reset the remote. The reason could be the i2c connection to the MPU-9250 and some interference when rf transmitting. In the next step I will redesign the system and use an other arduino to check the MPU-9250 externally. The feedback to the main controller could be with just one wire discrete line.
I had also 2 freezes on my receiver after about 2 hours of continuous use. In one case it was during a pull action, the relays remained energized, wich also keep the pulling ongoing until i disconnected the power supply. Luckily nothing bad happen.
To prevent such conditions i have implemented an watchdog timer. To use this feature you will need to flash the arduino nanos with the optiboot bootloader and use them like an uno. If you don't flash optiboot you will get stuck in an bootloop after the WDT reset.


Credits and Thanks 

I am happy to use:
The "LightWS2812" library. It can be found under "ID159" in the PlatformIO library manager or https://github.com/cpldcpu/light_ws2812 by Tim (cpldcpu).

The "MPU9250_asukiaaa" library. It can be found under "ID1751" in the PlatformIO library manager or https://github.com/asukiaaa/MPU9250_asukiaaa by Asuki Kono.

The "RadioHead" library. It can be found under "ID124" in the PlatformIO library manager or http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.82.zip by Mike McCauley.

The "Polymorphic Buttons" library. It can be found under "ID1899" in the PlatformIO library manager or https://github.com/JCWentzel/PolymorphicButtons by JC Wentzel.

The "SparkFun MPU-9250 9 DOF IMU Breakout" library. It can be found under "ID944" in the PlatformIO library manager or https://github.com/sparkfun/MPU-9250_Breakout by SparkFun Electronics.

thank you!
