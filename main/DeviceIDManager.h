#ifndef DEVICEIDMANAGER_H
#define DEVICEIDMANAGER_H

#include <Arduino.h>

class DeviceIDManager {
  public:
    // Construtor
    DeviceIDManager();

    // Configura os pinos e lê o ID do dispositivo
    void begin();

    // Retorna o ID do dispositivo (valor entre 0 e 3)
    uint8_t getID();

  private:
    uint8_t _pinA;
    uint8_t _pinB;
    uint8_t _deviceID;
};

#endif
