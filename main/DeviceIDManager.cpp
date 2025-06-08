#include "DeviceIDManager.h"
#include "setup.h"  // Inclui as definições de pinos

DeviceIDManager::DeviceIDManager() : _pinA(ID_DEVICE_B0), _pinB(ID_DEVICE_B1), _deviceID(0) {
}

void DeviceIDManager::begin() {
  pinMode(_pinA, INPUT_PULLUP);
  pinMode(_pinB, INPUT_PULLUP);
  
  uint8_t a = digitalRead(_pinA);
  uint8_t b = digitalRead(_pinB);
  _deviceID = (a ? 1 : 0) + (b ? 2 : 0);
  
  Serial.print("ID do dispositivo lido: ");
  Serial.println(_deviceID);
}

uint8_t DeviceIDManager::getID() {
  return _deviceID;
}
