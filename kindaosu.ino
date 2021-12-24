#include <Arduino.h>
#include "Menu.h"

int main () {
  init ();  // initialize timers

  // TODO game master class which will contain
  // game state and menu and orchestrate all other components
  // atm everything is in menu which doesn't make much sense overall
  
  Menu menu;
  while (true) {
    menu.display();
  }
  
  Serial.flush (); // let serial printing finish
}
