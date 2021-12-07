#ifndef JOYSTICK_H
#define JOYSTICK_H
// TODO: singleton

class Joystick {
public:
  Joystick() {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    pinMode(swPin, INPUT_PULLUP); 
    joyMoved = false;
    buttonState = false;
    lastDebounceTime = millis();
  }

  int getStateX() { return getState(xPin); }
  int getStateY() { return getState(yPin); }
  int detectMoveX() { return detectMove(xPin); }
  int detectMoveY() { return detectMove(yPin); }

  bool getButton() {
    int reading = digitalRead(swPin);
    if (reading != lastButtonReading) {
      lastDebounceTime = millis();
    }
    lastButtonReading = reading;
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == 0) {
          return 1;
        }
      }
    }
    return 0;
  }
  
private:
  const int xPin = A1, yPin = A0, swPin = 2;
  const int minThreshold = 200, maxThreshold = 800;
  
  const int debounceDelay = 50;
  unsigned long lastDebounceTime;
  int lastButtonReading;

  bool joyMoved, buttonState;

  int detectMove(int pin) {
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
      return -1; 
    } else if (val <= minThreshold) {
      return 1;
    }
    return 0;
  }
};

#endif
