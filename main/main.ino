#include "setup.h"
#include "lamp.h"
#include "controller.h"
#include "system.h"
#include "command_processor.h"
#include "sample_data.h"
#include <CircularBuffer.hpp>
#include "metrics_calculator.h"

#include <SPI.h>
#include "mcp_can.h"
#include "CANFragmented.h"
#include "DeviceIDManager.h"
#include "ProtocolController.h"

String commands[2];
String responses[2];
uint8_t dest;

// ================== INSTÂNCIAS ==================
String inputString = "";    
bool stringComplete = false; 
unsigned long lastTime = 0;   // Guarda o tempo da última execução do controlador

Lamp lamp;
Controller controller;
System sys;
MetricsCalculator metrics;
CircularBuffer<SampleData, SIZE_CIRCULAR_BUFFER> sampleBuffer;
CommandProcessor commandprocessor(&lamp, &controller, &sys, &sampleBuffer, &metrics);


// Flag para controlar o envio do comando apenas uma vez enquanto o dispositivo possui o token.
bool commandSent = false;
// Instancia os objetos usando os valores configurados em setup.h
CANFragmented canFrag(CS_PIN, CAN_BAUDRATE, MCP_CLOCK, CAN_ID, MAX_SIZE_DATA);
DeviceIDManager devManager;
ProtocolController protocol(&canFrag, &devManager, &commandprocessor);

// ================== SETUP ==================
void setup() {
  Serial.begin(BAUD_RATE);
  //while (!Serial);  // Aguarda pela conexão serial
  Serial.println("Digite um comando e pressione ENTER:");
  controller.calibrate(&lamp);
  // Inicializa o núcleo 1 para rodar tarefas em paralelo

  /*CAN BUS*/
  protocol.begin();
  // Configura o LED onboard (no Raspberry Pi Pico, LED_BUILTIN normalmente é o pino 25).
  pinMode(LED_BUILTIN, OUTPUT);
  // Inicializa a semente para números aleatórios (dependendo do hardware pode ser necessário usar outro pino)
  randomSeed(analogRead(0));
  multicore_launch_core1(core1Task);
  Serial.println("Ready!");
}
