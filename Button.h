#ifndef BUTTON_H
#define BUTTON_H

class Button {
public:

  Button(int p): buttonPin(p) {
    // all buttons are connected via input_pullup
    pinMode(buttonPin, INPUT_PULLUP);
    lastDebounceTime = millis();
    buttonState = 1; // pullup so idle is 1
    lastButtonReading = 1;
  }

  /*
   * negate the state received from the update
   * because we're using pullup
   */
  bool getState() {
    updateState();
    return !buttonState;
  }

  bool getPress() {
    bool lastState = buttonState;
    updateState();
    // we don't care if it was already pressed
    if (lastState == 0) {
      return false;
    }
    return !buttonState;
  }

private:
  
  const byte buttonPin;
  unsigned long lastDebounceTime;
  bool lastButtonReading, buttonState;

  /*
   * after two consecutive readings of the same value
   * sepparated by the DEBOUNCE_DELAY,
   * if the reading is different from the button state
   * I mark the state change accordingly
   */
  void updateState() {
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonReading) {
      lastDebounceTime = millis();
    }
    lastButtonReading = reading;
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttonState) {
        buttonState = reading;
      }
    }
  }
};

#endif
