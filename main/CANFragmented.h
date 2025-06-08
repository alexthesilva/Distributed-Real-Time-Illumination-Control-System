#ifndef CANFRAGMENTED_H
#define CANFRAGMENTED_H

#include <Arduino.h>
#include <SPI.h>
#include "mcp_can.h"

class CANFragmented {
  public:
    // Construtor:
    // csPin: Pino Chip Select para o MCP2515
    // canBaudrate: Baud rate do barramento CAN
    // mcpClock: Frequência do cristal do MCP2515
    // canId: ID CAN para as mensagens fragmentadas
    // maxDataSize: Tamanho máximo do buffer de recepção
    // terminator: Caractere que indica o fim da mensagem longa
    CANFragmented(byte csPin, long canBaudrate = CAN_500KBPS, int mcpClock = MCP_8MHZ,
                  unsigned long canId = 0x321, int maxDataSize = 640, char terminator = '$');

    // Inicializa a interface CAN (inicia SPI e o MCP2515)
    bool begin();

    // Envia uma mensagem longa, fragmentada em frames de 8 bytes
    void sendLargeMessage(const byte* data, int length);

    // Processa um frame recebido e reconstroi a mensagem
    void processReceivedFrame(byte* rxBuf, byte rxLen);

    // Checa se há novos frames e os processa (para chamar no loop())
    bool update();

    // Retorna a mensagem completa (se disponível) e a limpa
    String getCompleteMessage();

  private:
    MCP_CAN can;              // Instância do objeto MCP_CAN
    byte _csPin;
    long _canBaudrate;
    int _mcpClock;
    unsigned long _canId;
    int _maxDataSize;
    char _terminator;

    // Buffer e variáveis de estado para recepção
    byte* receivedData;
    int totalReceived;
    int expectedFrameIndex;

    // Armazena a mensagem completa assim que terminada
    String _completeMessage;
};

#endif // CANFRAGMENTED_H
