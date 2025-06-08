#include "CANFragmented.h"

CANFragmented::CANFragmented(byte csPin, long canBaudrate, int mcpClock,
                             unsigned long canId, int maxDataSize, char terminator)
  : can(csPin),
    _csPin(csPin),
    _canBaudrate(canBaudrate),
    _mcpClock(mcpClock),
    _canId(canId),
    _maxDataSize(maxDataSize),
    _terminator(terminator),
    totalReceived(0),
    expectedFrameIndex(0),
    _completeMessage("")
{
  receivedData = new byte[_maxDataSize];
}

bool CANFragmented::begin() {
  SPI.begin();
  if (can.begin(MCP_ANY, _canBaudrate, _mcpClock) == CAN_OK) {
    can.setMode(MCP_NORMAL);
    return true;
  } else {
    return false;
  }
}

void CANFragmented::sendLargeMessage(const byte* data, int length) {
  int frameIndex = 0;
  int offset = 0;
  
  while (offset < length) {
    byte canBuffer[8];
    canBuffer[0] = frameIndex;
    
    int chunkSize = length - offset;
    if (chunkSize > 7) chunkSize = 7;
    
    // Copia os dados para o frame
    for (int i = 0; i < chunkSize; i++) {
      canBuffer[i + 1] = data[offset + i];
    }
    
    // Preenche o restante com zeros
    for (int i = chunkSize + 1; i < 8; i++) {
      canBuffer[i] = 0;
    }
    
    byte sendStatus = can.sendMsgBuf(_canId, 0, 8, canBuffer);
    if (sendStatus == CAN_OK) {
      //Serial.print("Enviado frameIndex=");
     // Serial.print(frameIndex);
     // Serial.print(" (");
     // Serial.print(chunkSize);
     // Serial.println(" bytes).");
    } else {
      Serial.println("Erro ao enviar frame!");
    }
    
    offset += chunkSize;
    frameIndex++;
  }
  
  // Envia o frame de terminação com o caractere terminador
  byte termBuffer[8];
  termBuffer[0] = frameIndex;
  termBuffer[1] = _terminator;
  for (int i = 2; i < 8; i++) {
    termBuffer[i] = 0;
  }
  
  can.sendMsgBuf(_canId, 0, 8, termBuffer);
  //Serial.println("Frame de terminação enviada.");
}

void CANFragmented::processReceivedFrame(byte* rxBuf, byte rxLen) {
  byte frameIndex = rxBuf[0];
  
  if (frameIndex == expectedFrameIndex) {
    for (int i = 1; i < rxLen; i++) {
      if (rxBuf[i] == _terminator) { // Terminador encontrado; mensagem completa.
        if (totalReceived < _maxDataSize) {
          receivedData[totalReceived] = '\0';
        }
        _completeMessage = String((char*)receivedData);
        //Serial.print("Mensagem completa recebida: ");
        //Serial.println(_completeMessage);
        // Reinicia o buffer e os índices para a próxima mensagem.
        totalReceived = 0;
        expectedFrameIndex = 0;
        return;
      } else if (totalReceived < _maxDataSize - 1) {
        receivedData[totalReceived++] = rxBuf[i];
      }
    }
    expectedFrameIndex++;
  } else {
    Serial.print("Frame fora de ordem: esperava ");
    Serial.print(expectedFrameIndex);
    Serial.print(", recebeu ");
    Serial.println(frameIndex);
  }
}

bool CANFragmented::update() {
  if (can.checkReceive() == CAN_MSGAVAIL) {
    unsigned long rxId;
    byte rxLen;
    byte rxBuf[8];
    
    can.readMsgBuf(&rxId, &rxLen, rxBuf);
    
    if (rxId == _canId) {
      processReceivedFrame(rxBuf, rxLen);
    }
    return true;
  }
  return false;
}

String CANFragmented::getCompleteMessage() {
  String msg = _completeMessage;
  _completeMessage = "";
  return msg;
}
