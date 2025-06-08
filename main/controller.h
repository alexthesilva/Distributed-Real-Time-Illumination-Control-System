#ifndef CONTROLLER_H
#define CONTROLLER_H

class Lamp;

class Controller {
private:
    float G;               // Ganho da calibração (lux por % duty)
    float d;               // Offset da calibração (lux)
    float integratorState = 0; // Estado acumulado para o termo integral

    float previous_y = 0.0f;         // Armazena a última medida para calcular a derivada
    float filtered_derivative = 0.0f;

    bool manual_mode = false;
    float manual_duty = 0.0f;

    float actual_set_point_lux = 0;
    bool actual_anti_windup_state = true;
    bool actual_feedback_state = true;

    Lamp* lamp;            // Ponteiro para o objeto Lamp associado

public:
    // Construtor
    Controller();

    // Predição da iluminância baseada no duty cycle: L = d + G * duty
    float predict_lux(float duty);

    // Função de calibração usando um objeto Lamp
    void calibrate(Lamp* lamp);

    // Iteração do controlador integral:
    // r: referência de lux; measured_y: lux medido.
    // Retorna o sinal de controle (duty cycle, 0–100)
    float integral_controller(float r, float measured_y);
    float pid_controller(float r, float y);

    // Getter para o ganho G
    float getG();

    // Getter e Setter para o set-point
    float getSetPointLux();
    void setSetPointLux(float value);

    // Getters e setters para anti-windup e feedback
    bool getAntiWindupState();
    void setAntiWindupState(bool state);
    bool getFeedBackState();
    void setFeedBackState(bool state);

    // Retorna a iluminância externa: lux medido menos a contribuição do LED
    float getExternalIlluminance();

    void setManualMode(bool mode); //ativa/desativa modo manual
    bool getManualMode();          //retorna estado do modo manual
    void setManualDuty(float duty);//define duty cycle manual
    float getManualDuty();
};

#endif // CONTROLLER_H
