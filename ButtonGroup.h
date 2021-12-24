#ifndef BUTTON_GROUP_H
#define BUTTON_GROUP_H

#include "Button.h"
#include "Globals.h"

class ButtonGroup {
public:
  
  ButtonGroup(ButtonGroup &other) = delete;
  ButtonGroup operator=(const ButtonGroup &) = delete;
  
  static ButtonGroup* getInstance();

  /*
   * receive an array corresponding to the 
   * state of every button in the group
   * fill it with the updated states
   */
  void updateAllStates(bool *states) {
    for (int i = 0; i < buttonCount; ++i) {
      states[i] = buttons[i].getState();
    }
  }
  
  static const int buttonCount = 4;
  
private:

  ButtonGroup() {}

  static ButtonGroup* instance;
  
  Button buttons[buttonCount] = {
    Button(buttonGroupButton1Pin),
    Button(buttonGroupButton2Pin),
    Button(buttonGroupButton3Pin),
    Button(buttonGroupButton4Pin)
  };
};

static ButtonGroup* ButtonGroup::instance = nullptr;

static ButtonGroup* ButtonGroup::getInstance() {
  if (!instance) {
    instance = new ButtonGroup();
  }
  return instance;
}

#endif
