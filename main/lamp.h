#ifndef __LAMP_H__
#define __LAMP_H__

#include "setup.h"

class Lamp {
private:
    int pin_led;
    int pin_ldr;
    float actual_lux;
    float actual_duty = 0;
    float actual_voltage;
    bool occupancy;
    // Parâmetros adicionais para o estágio 2
    float occupiedLowerBound;
    float unoccupiedLowerBound;
    float energyCost;

public:
    // Construtor
    Lamp();

    // Getters
    float get_lux();
    float get_duty();
    float get_voltage_ldr();
    bool get_occupancy_state();
    float get_inst_power();

    // Setters
    void set_duty(float duty);
    void set_occupancy_state(bool occ);

    // Atualiza o valor da iluminância
    void update_lux();

    // Setters e getters para os parâmetros do estágio 2
    void setOccupiedLowerBound(float value);
    float getOccupiedLowerBound();
    void setUnoccupiedLowerBound(float value);
    float getUnoccupiedLowerBound();
    void setEnergyCost(float value);
    float getEnergyCost();
};

#endif
