#ifndef PROTOCOLCONTROLLER_H
#define PROTOCOLCONTROLLER_H

#include <Arduino.h>
#include "CANFragmented.h"
#include "DeviceIDManager.h"
#include "command_processor.h"
#include "setup.h"  // Para usar RETRIES e RETRANSMIT_INTERVAL

class ProtocolController {
  public:
    ProtocolController(CANFragmented* canFrag, DeviceIDManager* devManager, CommandProcessor* commandprocessor);

    void begin();
    void update();

    // Envia uma mensagem de comando (prefixo "CMD")
    void sendMessage(uint8_t targetID, String message);

    // Envia uma resposta (prefixo "RESP")
    void responseMessage(uint8_t targetID, String message);

    // Retorna a última mensagem (payload) recebida e limpa a variável interna
    String receiveMessage();

    // Passa o token para o próximo dispositivo
    void passToken(uint8_t nextDeviceID);

    // Retorna se este dispositivo possui o token
    bool hasToken();

    // Processa comandos vindos do Serial Monitor para testes
    void processSerialInput(String input);

  private:
    CANFragmented* _canFrag;
    DeviceIDManager* _devManager;
    CommandProcessor* _commandprocessor;
    uint8_t _myID;
    bool _hasToken;
    String _lastMessage; // armazena a última mensagem recebida (CMD ou RESP)

    // >>> Variáveis para gerenciar retransmissão <<<
    bool _waitingForAck;         // Se estamos aguardando uma resposta (RESP)
    unsigned long _lastSendTime; // Momento da última transmissão
    int _retryCount;             // Quantas vezes já retransmitimos
    uint8_t _pendingTargetID;    // Para quem enviamos a última CMD
    String _pendingCommand;      // Comando pendente aguardando ACK

    // Processa a mensagem recebida e faz a lógica de comando/resposta/token
    void handleReceivedMessage(String message);
};

#endif // PROTOCOLCONTROLLER_H
