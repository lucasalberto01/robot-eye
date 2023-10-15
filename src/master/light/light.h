#ifndef LIGHT_H
#define LIGHT_H

class Light {
   private:
    bool lightOn;

   public:
    Light(/* args */);

    // Turn the light on or off
    void toggle();

    // Check if the light is on
    bool isOn();

    // Set the light on or off
    void set(bool on);
};

#endif