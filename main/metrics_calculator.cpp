#include "metrics_calculator.h"
#include "setup.h"  // Define MAX_POWER, etc.
#include <math.h>

MetricsCalculator::MetricsCalculator(){
}

void MetricsCalculator::update(float y, float u, unsigned long current_time, float reference_L) {
    // Atualiza energia (integração do duty cycle ao longo do tempo)
    if (!_first_sample) {
        float delta_time = (current_time - _prev_time) / 1000000.0f; // converter us para segundos
        _energy += _prev_duty * delta_time;
    }

    // Cálculo do flicker usando duty cycle somente se:
    // 1) Não for a primeira amostra e
    // 2) A referência estiver estável.
    if (!_first_sample) {
        // Para aplicar a fórmula é necessário ter 2 amostras anteriores:
        // fₖ = |dₖ - dₖ₋₁| + |dₖ₋₁ - dₖ₋₂|, se (dₖ - dₖ₋₁)*(dₖ₋₁ - dₖ₋₂) < 0;
        // caso contrário, fₖ = 0.
        float d_k   = u;              // duty cycle atual
        float d_km1 = _prev_duty;       // duty cycle anterior
        float d_km2 = _prev_duty2;      // duty cycle de duas iterações atrás

        float diff1 = d_k   - d_km1;
        float diff2 = d_km1 - d_km2;

        // Se houver inversão de sinal nas diferenças, acumula o flicker
        if (diff1 * diff2 < 0.0f) {
            float flick = fabs(diff1) + fabs(diff2);
            _flicker_sum += flick;
            _flicker_count++;
        }
        else{
          _flicker_sum += 0;
          _flicker_count++;
        }
    }

    // Atualiza os estados históricos para a próxima iteração
    _prev_duty2 = _prev_duty;
    _prev_duty  = u;
    _prev_time  = current_time;
    _prev_lux   = y;

    // Atualiza a referência para controle de transientes
    _prev_reference = reference_L;
    _first_reference = false;

    // Cálculo do erro de visibilidade: acumula a diferença sempre que a iluminância medida está abaixo da referência
    if (y < reference_L) {
        _visibility_sum += (reference_L - y);
    }
    _visibility_count++;

    _first_sample = false;
}

float MetricsCalculator::getFlicker() {
    // Retorna o valor médio do flicker acumulado
    return (_flicker_count > 0) ? (_flicker_sum / _flicker_count) : 0.0f;
}

float MetricsCalculator::getEnergy() {
    // Energia total em Joules considerando que MAX_POWER é a potência máxima do LED
    return MAX_POWER * _energy;
}

float MetricsCalculator::getVisibility() {
    // Retorna o erro médio de visibilidade
    return (_visibility_count > 0) ? (_visibility_sum / _visibility_count) : 0.0f;
}

void MetricsCalculator::reset() {
    _prev_duty = 0;
    _prev_duty2 = 0;
    _prev_time = 0;
    _energy = 0;
    _flicker_sum = 0;
    _flicker_count = 0;
    _visibility_sum = 0;
    _visibility_count = 0;
    _prev_lux = 0;
    _prev_reference = 0;
    _first_sample = true;
    _first_reference = true;
}
