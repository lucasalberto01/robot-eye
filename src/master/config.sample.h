#ifndef CONFIG_H
#define CONFIG_H

/*
 * ROBOT CONFIGURATION
 */

#define WIFI_SSID "<your wifi ssid>"
#define WIFI_PASS "<your wifi password>"

#define HOST_ADDR "<your ngrok host>"
#define PORT_ADDR 80

#define TIME_TO_SLEEP 60000  // 1 minute

/*
 * MOTOR CONFIGURATION
 */

// Motor 1
#define MOTOR_M1_IN1 0
#define MOTOR_M1_IN2 4
#define MOTOR_M1_EN 25

// Motor 2
#define MOTOR_M2_IN1 32
#define MOTOR_M2_IN2 33
#define MOTOR_M2_EN 26

/*
 * DISPLAY CONFIGURATION
 */

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW  // Change this to your hardware type
#define MAX_DEVICES 2

#define CS_PIN 5

/*
 * LIGHT CONFIGURATION
 */

#define LED_PIN 13

/*
 * CONTROL PANEL
 */

#define RESER_BTN 2

#endif  // CONFIG_H