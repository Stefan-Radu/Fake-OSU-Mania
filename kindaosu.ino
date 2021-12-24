#include <Arduino.h>
#include "MasterClass.h"

int main () {
  init ();  // initialize timers
  
  MasterClass master;
  while (true) {
    master.display();
  }
  
  Serial.flush (); // let serial printing finish
}
