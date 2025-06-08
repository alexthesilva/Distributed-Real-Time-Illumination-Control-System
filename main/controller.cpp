#include "controller.h"
#include "lamp.h"
#include "setup.h"
#include <Arduino.h>

Controller::Controller() {
}

float Controller::predict_lux(float duty) {
    return d + G * duty;
}

float Controller::integral_controller(float r, float y) {
    if (!actual_feedback_state) {
        // Feedback desligado: retorna duty fixo (feed-forward) e zera o integrador
        integratorState = 0.0f;  // Reseta o integrador
        return FF_GAIN * G_USED_SETPOINT / G * r; 
    }

    // Cálculos com feedback ligado
    float FF = FF_GAIN * G_USED_SETPOINT / G;
    float one_over_Ti = ONE_OVER_Ti * G_USED_SETPOINT / G;
    float error = r - y;
    integratorState += error * one_over_Ti;

    float u_unsat = FF * r + integratorState;
    float u_sat = u_unsat;

    // Saturação
    u_sat = (u_sat > 100.0f) ? 100.0f : (u_sat < 0.0f) ? 0.0f : u_sat;

    // Anti-windup condicional
    if (actual_anti_windup_state) {
        float antiWindupGain = ANTI_WINDUP_GAIN * G_USED_SETPOINT / G; // Calculado apenas se necessário
        integratorState += antiWindupGain * (u_sat - u_unsat);
    }

    return u_sat;
}
float Controller::pid_controller(float r, float y) {
    if (!actual_feedback_state) {
        // Feedback desligado: retorna duty fixo (feed-forward) e zera os estados
        integratorState = 0.0f;  // Reseta o integrador
        previous_y = y;          // Inicializa a última medida para derivada
        return FF_GAIN * G_USED_SETPOINT / G * r;
    }

    // Cálculos com feedback ligado
    float FF = FF_GAIN * G_USED_SETPOINT / G;
    float one_over_Ti = ONE_OVER_Ti * G_USED_SETPOINT / G;
    float error = r - y;
    
    // Atualiza o integrador
    integratorState += error * one_over_Ti;

    // Termo derivativo simples (derivada da saída)
    // Note que não há divisão por dt, assumindo que o tempo de amostragem é constante.
    float derivativeTerm = -D_GAIN * (y - previous_y);
    previous_y = y;  // Atualiza a última medida para a próxima iteração

    // Combina os termos (feed-forward, integral e derivativo)
    float u_unsat = FF * r + integratorState + derivativeTerm;
    float u_sat = u_unsat;

    // Saturação: limita o sinal entre 0 e 100
    u_sat = (u_sat > 100.0f) ? 100.0f : (u_sat < 0.0f) ? 0.0f : u_sat;

    // Anti-windup condicional: ajusta o integrador se houver saturação
    if (actual_anti_windup_state) {
        float antiWindupGain = ANTI_WINDUP_GAIN * G_USED_SETPOINT / G;
        integratorState += antiWindupGain * (u_sat - u_unsat);
    }

    return u_sat;
}


void Controller::calibrate(Lamp* lamp) {
    this->lamp = lamp;  // Armazena o ponteiro para uso posterior
    float lux_0, lux_1;
    
    // Define duty cycle para 0%
    lamp->set_duty(0);
    delay(2000);
    lamp->update_lux();
    lux_0 = lamp->get_lux();

    // Define duty cycle para 100%
    lamp->set_duty(100);
    delay(2000);
    lamp->update_lux();
    lux_1 = lamp->get_lux();

    G = (lux_1 - lux_0) / 100.0;
    d = lux_0;

    lamp->set_duty(0); // Desliga o LED para estabilização
    delay(2000);
    lamp->update_lux();
}

float Controller::getG() {
    return G;
}

float Controller::getSetPointLux(){
  return actual_set_point_lux;
}

void Controller::setSetPointLux(float value){
  actual_set_point_lux = value;
}

bool Controller::getAntiWindupState(){
  return actual_anti_windup_state;
}

void Controller::setAntiWindupState(bool state) {
  actual_anti_windup_state = state;
}

bool Controller::getFeedBackState(){
  return actual_feedback_state;
}

void Controller::setFeedBackState(bool state) {
  actual_feedback_state = state;
}

float Controller::getExternalIlluminance(){
  if(lamp != nullptr)
    return lamp->get_lux() - G * lamp->get_duty();
  else
    return 0;
}

bool Controller::getManualMode() {
  return manual_mode;
}

float Controller::getManualDuty() {
  return  manual_duty;
}

void Controller::setManualMode(bool mode) {
  manual_mode = mode;
}

void Controller::setManualDuty(float duty) {
  manual_duty = duty;
}

