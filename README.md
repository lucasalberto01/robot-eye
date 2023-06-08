# Robot-eye 🤖

<p align="center">
  <img width="460" height="auto" src="https://github.com/lucasalberto01/robot-eye/assets/29151352/0019a6b6-f43f-4883-b67c-b0a6fffccd30">
</p>

This GitHub project consists of a control system for a mobile robot equipped with an ESP32, a MAX7219 LED display and a 3s2p battery. The main objective is to allow controlling the robot in an intuitive and fun way, using features such as animated eyes, speed control and display of texts in the eyes.

In open field tests, we were able to control it at a distance of 100 meters

## Hardware/Software

* PlatformIO
* Esp32
* Led MAX7219
* Baterry 3s2p
* Antenna 9dbi

## Features

* Real time controller
* Change eye types
* Control the two treadmills
* Control movement speed
* Display texts in eyes
* Two types of joystick for control
* FVP (option)
* Change FPV resolution (option)
* Controller with Wi-Fi
* Autonomy of more than 6 hours

### Future feature

* Encoder in motors
* SLAM

## Challenges

One of the main challenges was the interference it experiences from other 2.4 GHz networks. This leads to high latency at certain times, which can result in a loss of control.

## Conclusion

This project offers a complete platform for controlling a mobile robot with additional features such as animated eyes and the possibility of customization. It's a great choice for robotics and programming enthusiasts who want to experiment and explore the potential of the ESP32 in remotely controlled robot designs.
