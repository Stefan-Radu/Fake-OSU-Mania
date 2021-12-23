#ifndef MASTER_H
#define MASTER_H

class Master {
public:
  Master(Master &other) = delete;
  Master operator=(const Master &) = delete;

  static Master* getInstance();
  
  void selectSongTransmission(int song) {
    Wire.beginTransmission(SLAVE_NUMBER);
    Wire.write(song);
    Wire.endTransmission();
  }

private:

  Master() {
    Wire.begin();
  }

  static Master* instance;
};

Master* Master::instance = nullptr;

Master* Master::getInstance() {
  if (instance == nullptr) {
    instance = new Master();
  }
  return instance;
}

#endif
