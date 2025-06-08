#ifndef METRICS_CALCULATOR_H
#define METRICS_CALCULATOR_H

class MetricsCalculator {
public:
    MetricsCalculator();
    
    // Atualiza as medições com:
    // y: iluminância medida (Lux)
    // u: duty cycle atual (valor entre 0 e 1)
    // current_time: tempo atual em milissegundos
    // reference_L: referência de iluminância (Lux)
    void update(float y, float u, unsigned long current_time, float reference_L);
    
    // Retorna o valor médio do flicker acumulado (s⁻¹)
    float getFlicker();
    
    // Retorna a energia acumulada (Joules)
    float getEnergy();
    
    // Retorna o erro médio de visibilidade (Lux)
    float getVisibility();
    
    // Reseta todos os acumuladores e estados internos
    void reset();

private:
    // Estado para energia
    float _prev_duty = 0;         // duty cycle da amostra anterior (dₖ₋₁)
    float _prev_duty2 = 0;        // duty cycle da penúltima amostra (dₖ₋₂)
    unsigned long _prev_time = 0;
    float _energy = 0;
    
    // Estado para flicker
    float _flicker_sum = 0;
    unsigned int _flicker_count = 0;
    
    // Estado para visibilidade
    float _visibility_sum = 0;
    unsigned int _visibility_count = 0;
    
    // Para detectar variações explícitas na referência de iluminância
    float _prev_reference = 0;
    bool _first_reference = true;

    // Outras variáveis (ex.: para histórico de iluminância, se necessário)
    float _prev_lux = 0;
    bool _first_sample = true;
};

#endif
