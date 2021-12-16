#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "Button.h"

class Joystick {
public:
  Joystick(Joystick &other) = delete;
  Joystick operator=(const Joystick &) = delete;

  static Joystick* getInstance();
  
  int getStateX() { return getState(xPin); }
  int getStateY() { return getState(yPin); }
  int detectMoveX() { return detectMove(xPin, joyMovedX); }
  int detectMoveY() { return detectMove(yPin, joyMovedY); }

  bool getButton() {
    return button.getPress();
  }
  
private:

  Joystick(): button(swPin) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    joyMovedX = false;
    joyMovedY = false;
  }

  static Joystick *instance;
  
  const int xPin = A1, yPin = A0, swPin = 2;
  const int minThreshold = 200, maxThreshold = 800;

  Button button;

  bool joyMovedX, joyMovedY;

  int detectMove(int pin, bool &joyMoved) {
    int state = getState(pin);
    
    if (state == 0) {
      joyMoved = false;
      return 0;
    }

    if (!joyMoved && state != 0) {
      joyMoved = true;
      return state;
    }
    return 0;
  }

  int getState(int pin) {
    int val = analogRead(pin);
    if (val >= maxThreshold) {
      return 1; 
    } else if (val <= minThreshold) {
      return -1;
    }
    return 0;
  }
};

Joystick* Joystick::instance = nullptr;

Joystick* Joystick::getInstance() {
  if (instance == nullptr) {
    instance = new Joystick();
  }
  return instance;
}

#endif
