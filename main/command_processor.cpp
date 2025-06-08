#include "command_processor.h"
#include "lamp.h"
#include "controller.h"
#include "system.h"
#include <Arduino.h>

CommandProcessor::CommandProcessor(Lamp* lamp, Controller* controller, System* sys, CircularBuffer<SampleData, SIZE_CIRCULAR_BUFFER>* sampleBuffer, MetricsCalculator* metrics) {
  this->lamp = lamp;
  this->controller = controller;
  this->sys = sys;
  this->sampleBuffer = sampleBuffer;
  this->metrics = metrics; 
}

String CommandProcessor::sendAck() {
  return "ack";
}

String CommandProcessor::sendError() {
  return "err";
}

String CommandProcessor::processCommand(String cmd) {
  String result = "";

  // Tokeniza o comando (máximo 10 tokens)
  const int maxTokens = 10;
  String tokens[maxTokens];
  int tokenCount = 0;

  size_t startIndex = 0;
  int spaceIndex = cmd.indexOf(' ');
  while (spaceIndex != -1 && tokenCount < maxTokens) {
    tokens[tokenCount++] = cmd.substring(startIndex, spaceIndex);
    startIndex = spaceIndex + 1;
    spaceIndex = cmd.indexOf(' ', startIndex);
  }
  if (startIndex < cmd.length() && tokenCount < maxTokens) {
    tokens[tokenCount++] = cmd.substring(startIndex);
  }

  if (tokenCount == 0) {
    return "Nenhum comando encontrado.";
  }

  // Se o primeiro token for "g", trata-se de um comando GET
  if (tokens[0] == "g") {
    if (tokenCount < 3) {
      return "Comando GET inválido: falta índice ou parâmetro.";
    }
    String subcmd = tokens[1];
    String index = tokens[2];
    if (subcmd == "u") {  // Duty cycle
      float value = lamp->get_duty();
      result = "u " + index + " " + String(value);
    } else if (subcmd == "r") {  // Set-point
      float value = controller->getSetPointLux();
      result = "r " + index + " " + String(value);
    } else if (subcmd == "y") {  // Medida de lux
      float value = lamp->get_lux();
      result = "y " + index + " " + String(value);
    } else if (subcmd == "v") {  // Tensão no LDR
      float value = lamp->get_voltage_ldr();
      result = "v " + index + " " + String(value);
    } else if (subcmd == "o") {  // Estado de ocupação
      bool value = lamp->get_occupancy_state();
      result = "o " + index + " " + String(value);
    } else if (subcmd == "a") {  // Estado anti-windup
      bool value = controller->getAntiWindupState();
      result = "a " + index + " " + String(value);
    } else if (subcmd == "f") {  // Estado feedback
      bool value = controller->getFeedBackState();
      result = "f " + index + " " + String(value);
    } else if (subcmd == "d") {  // Iluminância externa
      float value = controller->getExternalIlluminance();
      result = "d " + index + " " + String(value);
    } else if (subcmd == "p") {  // Potência instantânea
      float value = lamp->get_inst_power();
      result = "p " + index + " " + String(value);
    } else if (subcmd == "t") {  // Tempo decorrido
      float value = sys->getElapsedSeconds();
      result = "t " + index + " " + String(value);
    } else if (subcmd == "b") {  // Buffer de dados
      if (tokenCount >= 4) {
        String varName = tokens[2];
        String id = tokens[3];
        String response = "b " + varName + " " + id + " ";
        for (int i = 0; i < sampleBuffer->size(); i++) {
          SampleData sample = (*sampleBuffer)[i];
          float value = 0.0;
          if (varName == "y") {
            value = sample.y;
          } else if (varName == "u") {
            value = sample.u;
          } else {
            return sendError();
          }
          response += String(value, 2);
          if (i < sampleBuffer->size() - 1) {
            response += ",";
          }
        }
        result = response;
      } else {
        result = sendError();
      }
    } else if (subcmd == "m") {  // Estado do modo manual
      bool mode = controller->getManualMode();
      result = "m " + index + " " + String(mode);
    } else if (subcmd == "E") {  // Energia consumida
      float energy = metrics->getEnergy();
      result = "E " + index + " " + String(energy, 2); 
    } else if (subcmd == "V") {  // Erro de visibilidade
      float visibility = metrics->getVisibility();
      result = "V " + index + " " + String(visibility, 2);
    } else if (subcmd == "F") {  // Flicker
      float flicker = metrics->getFlicker();
      result = "F " + index + " " + String(flicker, 2);
    } else if (subcmd == "O") {  // Limite para estado OCCUPIED
      float value = lamp->getOccupiedLowerBound();
      result = "O " + index + " " + String(value);
    } else if (subcmd == "U") {  // Limite para estado UNOCCUPIED
      float value = lamp->getUnoccupiedLowerBound();
      result = "U " + index + " " + String(value);
    } else if (subcmd == "L") {  // Limite de iluminância (placeholder)
      // Para exemplo, retorna o mesmo que O
      float value = lamp->getOccupiedLowerBound();
      result = "L " + index + " " + String(value);
    } else if (subcmd == "C") {  // Custo de energia
      float value = lamp->getEnergyCost();
      result = "C " + index + " " + String(value);
    } else {
      result = "GET comando desconhecido: " + subcmd;
    }
  } else {
    // Comandos SET
    String command = tokens[0];
    if (command == "u") {  // Define duty cycle
      if (tokenCount >= 3) {
        float value = tokens[2].toFloat();
        controller->setManualMode(true);
        controller->setManualDuty(value);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "r") {  // Define referência de lux
      if (tokenCount >= 3) {
        float val = tokens[2].toFloat();
        controller->setSetPointLux(val);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "o") {  // Define estado de ocupação
      if (tokenCount >= 3) {
        int val = tokens[2].toInt();
        lamp->set_occupancy_state(val != 0);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "a") {  // Define estado anti-windup
      if (tokenCount >= 3) {
        int val = tokens[2].toInt();
        controller->setAntiWindupState(val != 0);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "f") {  // Define estado feedback
      if (tokenCount >= 3) {
        int val = tokens[2].toInt();
        controller->setFeedBackState(val != 0);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "s") {  // Inicia stream de variável
      if (tokenCount >= 3) {
        result = "Stream iniciado para variável " + tokens[1] + " do desk " + tokens[2];
      } else {
        result = sendError();
      }
    } else if (command == "m") {  // Ativa/desativa o controle manual
      if (tokenCount >= 3) {
        // Aqui, espera-se que tokens[2] seja "true" ou "false"
        String val = tokens[2];
        bool mode = (val == "true");
        controller->setManualMode(mode);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "S") {  // Para stream
      if (tokenCount >= 3) {
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "O") {  // Define limite para estado OCCUPIED
      if (tokenCount >= 3) {
        float val = tokens[2].toFloat();
        lamp->setOccupiedLowerBound(val);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "U") {  // Define limite para estado UNOCCUPIED
      if (tokenCount >= 3) {
        float val = tokens[2].toFloat();
        lamp->setUnoccupiedLowerBound(val);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "C") {  // Define custo de energia
      if (tokenCount >= 3) {
        float val = tokens[2].toFloat();
        lamp->setEnergyCost(val);
        result = sendAck();
      } else {
        result = sendError();
      }
    } else if (command == "R") {  // Reinicia o sistema
      result = sendAck();
      sys->doReset();
    }  else if (command == "Z") {  // Reset das métricas
    if (tokenCount >= 2) {
      metrics->reset();
      result = sendAck();
    } else {
      result = sendError();
    }
  }else {
      result = "Comando SET desconhecido: " + command;
    }
  }

  return result;
}

void CommandProcessor::sendResponse(String response) {
  Serial.println(response);
}
