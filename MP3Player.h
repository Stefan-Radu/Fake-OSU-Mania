#ifndef MP3PLAYER_H
#define MP3PLAYER_H

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

class MP3Player {
public:
  MP3Player(MP3Player &other) = delete;
  MP3Player operator=(const MP3Player &) = delete;

  static MP3Player* getInstance(int);

  void play() {
    mp3Player.play(1);
  }

  void stop() {
    mp3Player.stop();
  }
  
private:

  MP3Player(int v = 6):mp3Serial(rxPin, txPin), volume(v) {
    mp3Serial.begin(9600);
    // initialize serial communication
    if (!mp3Player.begin(mp3Serial)) {
      Serial.println("mp3 seems not to work lol =))) check sd card or smth");
      while(true);
    }
    mp3Player.volume(volume);  // 0 - 30
  }

  SoftwareSerial mp3Serial;
  DFRobotDFPlayerMini mp3Player;

  static MP3Player *instance;
  const int rxPin = 13, txPin = 3;
  int volume; // 0 - 30
};

MP3Player* MP3Player::instance = nullptr;

MP3Player* MP3Player::getInstance(int v = 3) {
  if (instance == nullptr) {
    instance = new MP3Player(v);
  }
  return instance;
}

#endif
