#include "lamp.h"
#include "setup.h"
#include <Arduino.h>

// Construtor
Lamp::Lamp() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  set_duty(0);
  occupancy = false;
  occupiedLowerBound = 0;
  unoccupiedLowerBound = 0;
  energyCost = 0;
  
  delay(200);
  update_lux();
}

void Lamp::update_lux() {
  float total_lux = 0;
  // Toma 20 amostras e calcula a média
  for (int i = 0; i < 20; i++) {
      int adcValue = analogRead(LDR_PIN);
      // Previne divisão por zero
      if(adcValue == 0) {
          continue;
      }
      actual_voltage = adcValue * (VCC / 4095.0);
      // Fórmula: R_LDR = R2 * (VCC / Vout - 1)
      float R_LDR = R2 * (VCC / actual_voltage - 1.0);
      // Calcula lux: lux = (R0 / R_LDR)^(1/gammaVal)
      total_lux += pow(R0 / R_LDR, 1.0 / gammaVal);
     // delayMicroseconds(5000);
  }
  actual_lux = total_lux / 20.0;
}

void Lamp::set_duty(float duty) {
  if (duty < 0) duty = 0;
  if (duty > 100) duty = 100;

  actual_duty = duty;
  analogWrite(LED_PIN, map(duty, 0, 100, 0, 255));
}

float Lamp::get_duty() {
  return actual_duty;
}

float Lamp::get_lux() {
  return actual_lux;
}

float Lamp::get_voltage_ldr(){
  return actual_voltage;
}

bool Lamp::get_occupancy_state(){
  return occupancy;
}

void Lamp::set_occupancy_state(bool occ) {
  occupancy = occ;
}

float Lamp::get_inst_power(){
  // Assume que actual_duty é uma porcentagem (0 a 100)
  return MAX_POWER * actual_duty / 100.0;
}

// Implementações para os parâmetros do estágio 2
void Lamp::setOccupiedLowerBound(float value) {
  occupiedLowerBound = value;
}

float Lamp::getOccupiedLowerBound() {
  return occupiedLowerBound;
}

void Lamp::setUnoccupiedLowerBound(float value) {
  unoccupiedLowerBound = value;
}

float Lamp::getUnoccupiedLowerBound() {
  return unoccupiedLowerBound;
}

void Lamp::setEnergyCost(float value) {
  energyCost = value;
}

float Lamp::getEnergyCost() {
  return energyCost;
}
