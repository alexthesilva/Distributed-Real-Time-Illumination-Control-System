#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include "setup.h"
#include <CircularBuffer.hpp>
#include "sample_data.h"
#include "metrics_calculator.h"
#include <Arduino.h> 

class Lamp;
class Controller;
class System;

class CommandProcessor {
private:
    Lamp* lamp;
    Controller* controller;
    System* sys;
    MetricsCalculator* metrics;
    // Declare sampleBuffer como ponteiro para CircularBuffer
    CircularBuffer<SampleData, SIZE_CIRCULAR_BUFFER>* sampleBuffer;

    float actual_set_point_lux = 0;
    
    String sendAck();
    String sendError();

public:
    // Construtor com os parâmetros necessários
    CommandProcessor(Lamp* lamp,
                     Controller* controller,
                     System* system,
                     CircularBuffer<SampleData, SIZE_CIRCULAR_BUFFER>* sampleBuffer, MetricsCalculator* metrics);

    void sendResponse(String response);
    String processCommand(String cmd);
};

#endif
