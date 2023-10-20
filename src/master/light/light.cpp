#include "light.h"

#include <Arduino.h>

#include "../config.h"

Light::Light() {
    // Set LED pin to output
    pinMode(LED_PIN, OUTPUT);

    // Set initial light state to OFF
    digitalWrite(LED_PIN, LOW);
}

// Turn the light on or off
void Light::toggle() {
    lightOn = !lightOn;
    digitalWrite(LED_PIN, lightOn);
}

// Check if the light is on
bool Light::isOn() {
    return lightOn;
}

// Set the light on or off
void Light::set(bool on) {
    lightOn = on;
    digitalWrite(LED_PIN, lightOn);
}