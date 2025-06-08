#include "ProtocolController.h"

ProtocolController::ProtocolController(CANFragmented* canFrag,
                                       DeviceIDManager* devManager,
                                       CommandProcessor* commandprocessor)
  : _canFrag(canFrag),
    _devManager(devManager),
    _commandprocessor(commandprocessor),
    _hasToken(false),
    _lastMessage(""),
    // Inicialização das variáveis de retransmissão
    _waitingForAck(false),
    _lastSendTime(0),
    _retryCount(0),
    _pendingTargetID(0),
    _pendingCommand("")
{
}

void ProtocolController::begin() {
  _devManager->begin();
  _myID = _devManager->getID();
  Serial.print("Device ID: ");
  Serial.println(_myID);

  if (_canFrag->begin()) {
    Serial.println("CAN inicializado com sucesso.");
  } else {
    Serial.println("Erro ao inicializar o CAN!");
  }

  // Dispositivo 0 inicia com o token (exemplo).
  if (_myID == 0) {
    _hasToken = true;
    Serial.println("Token inicial atribuído a este dispositivo.");
  }
}

bool ProtocolController::hasToken() {
  return _hasToken;
}

void ProtocolController::sendMessage(uint8_t targetID, String message) {
  // Se não possuo token, não posso enviar comando.
  if (!_hasToken) {
    Serial.println("Não é possível enviar mensagem: token não disponível.");
    return;
  }

  // Se já estou aguardando ack de um comando anterior, não mando outro agora.
  if (_waitingForAck) {
    Serial.println("Já aguardando ACK de outra mensagem, aguarde.");
    return;
  }

  // Monta a mensagem: "CMD:dev:<destino>:from:<origem>:<message>"
  String msg = "CMD:dev:" + String(targetID) + ":from:" + String(_myID) + ":" + message;

  // Envia pelo CAN (fragmentado, se necessário)
  _canFrag->sendLargeMessage((const byte*)msg.c_str(), msg.length());

  // Armazena infos para possível retransmissão
  _waitingForAck = true;
  _lastSendTime = millis();
  _retryCount = 0;
  _pendingTargetID = targetID;
  _pendingCommand = message;

  Serial.println("Mensagem enviada (CMD). Aguarda resposta do dispositivo " + String(targetID));
}

void ProtocolController::responseMessage(uint8_t targetID, String message) {
  // Monta a resposta: "RESP:dev:<destino>:from:<origem>:<message>"
  String msg = "RESP:dev:" + String(targetID) + ":from:" + String(_myID) + ":" + message;
  _canFrag->sendLargeMessage((const byte*)msg.c_str(), msg.length());

  // Para respostas, normalmente não aguardamos ACK — mas se fosse necessário,
  // poderíamos implementar lógica de espera, de forma similar ao CMD.
  // Aqui não há round-robin para RESP: quem envia a CMD espera, recebe RESP e passa token.

  Serial.println("Resposta enviada: " + msg);
}

String ProtocolController::receiveMessage() {
  String ret = _lastMessage;
  _lastMessage = "";
  return ret;
}

void ProtocolController::passToken(uint8_t nextDeviceID) {
  if (!_hasToken) {
    Serial.println("Não é possível passar o token: dispositivo não o possui.");
    return;
  }
  // Monta a mensagem de token: "TKN:to:<destino>"
  String msg = "TKN:to:" + String(nextDeviceID);
  _canFrag->sendLargeMessage((const byte*)msg.c_str(), msg.length());

  // Perde o token imediatamente após passar.
  _hasToken = false;
  Serial.println("Token passado para o dispositivo " + String(nextDeviceID));
}

void ProtocolController::handleReceivedMessage(String message) {
  // Extrai o tipo da mensagem: "CMD", "RESP" ou "TKN"
  int idx = message.indexOf(':');
  if (idx == -1) return;

  String type = message.substring(0, idx);

  if (type == "CMD") {
    // Formato esperado: "CMD:dev:<destino>:from:<origem>:<command>"
    int devIndex = message.indexOf("dev:");
    int fromIndex = message.indexOf(":from:");
    if (devIndex == -1 || fromIndex == -1) return;

    String destStr = message.substring(devIndex + 4, fromIndex);
    uint8_t dest = destStr.toInt();
    int colonAfterFrom = message.indexOf(":", fromIndex + 6);
    if (colonAfterFrom == -1) return;

    String originStr = message.substring(fromIndex + 6, colonAfterFrom);
    uint8_t originID = originStr.toInt();
    String cmd = message.substring(colonAfterFrom + 1);

    // Se o comando é destinado a mim
    if (dest == _myID) {
      // Armazena info da última mensagem
      _lastMessage = "CMD de " + String(originID) + ": " + cmd;
      // Processa o comando e gera a resposta
      String return_cmd = _commandprocessor->processCommand(cmd);
      responseMessage(originID, return_cmd);
    }
  }
  else if (type == "RESP") {
    // Formato: "RESP:dev:<destino>:from:<origem>:<response>"
    int devIndex = message.indexOf("dev:");
    int fromIndex = message.indexOf(":from:");
    if (devIndex == -1 || fromIndex == -1) return;

    String destStr = message.substring(devIndex + 4, fromIndex);
    uint8_t dest = destStr.toInt();
    int colonAfterFrom = message.indexOf(":", fromIndex + 6);
    if (colonAfterFrom == -1) return;

    String originStr = message.substring(fromIndex + 6, colonAfterFrom);
    uint8_t originID = originStr.toInt();
    String resp = message.substring(colonAfterFrom + 1);

    // Se a resposta é para mim
    if (dest == _myID) {
      _lastMessage = "RESP de " + String(originID) + ": " + resp;

      // >>> Aqui entendemos que recebemos um ACK da nossa última CMD <<<
      if (_waitingForAck && (originID == _pendingTargetID)) {
        // Limpamos estado de retransmissão
        _waitingForAck = false;
        _retryCount = 0;
      }

      // Após receber resposta, passamos o token para o próximo dispositivo
      uint8_t nextToken = (_myID + 1) % TOTAL_DEVICES;
      passToken(nextToken);
    }
  }
  else if (type == "TKN") {
    // Formato: "TKN:to:<destino>"
    int toIndex = message.indexOf("to:");
    if (toIndex == -1) return;

    String destStr = message.substring(toIndex + 3);
    uint8_t dest = destStr.toInt();
    if (dest == _myID) {
      _hasToken = true;
      Serial.println("Token recebido neste dispositivo.");
    }
  }
  else {
    // Se for algum outro tipo de mensagem não esperado
    _lastMessage = message;
  }
}

void ProtocolController::update() {
  // Verifica se há mensagens completas recebidas via CAN
  _canFrag->update();
  String msg = _canFrag->getCompleteMessage();
  if (msg.length() > 0) {
    handleReceivedMessage(msg);
  }

  // >>> Lógica de retransmissão <<<
  if (_waitingForAck) {
    unsigned long now = millis();
    // Se já passou do intervalo de retransmissão
    if ((now - _lastSendTime) > RETRANSMIT_INTERVAL) {
      // Se ainda não atingiu o máximo de tentativas, retransmite
      if (_retryCount < RETRIES) {
        _retryCount++;
        _lastSendTime = now;

        // Monta novamente a mensagem para retransmitir
        String msgToResend = "CMD:dev:" + String(_pendingTargetID) + 
                             ":from:" + String(_myID) + 
                             ":" + _pendingCommand;

        _canFrag->sendLargeMessage((const byte*)msgToResend.c_str(), msgToResend.length());
        Serial.println("Retransmitindo CMD para dispositivo " + String(_pendingTargetID) +
                       " (tentativa " + String(_retryCount) + ")");
      } 
      else {
        // Excedeu o número de retransmissões
        Serial.println("Não foi possível receber ACK do dispositivo " 
                       + String(_pendingTargetID) + ". Passando token...");
        // Cancela estado de espera
        _waitingForAck = false;
        _retryCount = 0;

        // Passa o token para o próximo da fila
        uint8_t nextToken = (_myID + 1) % TOTAL_DEVICES;
        passToken(nextToken);
      }
    }
  }
}

void ProtocolController::processSerialInput(String input) {
  input.trim();
  if (input.startsWith("cmd")) {
    int firstSpace = input.indexOf(' ');
    if (firstSpace == -1) return;
    String remainder = input.substring(firstSpace + 1);
    int secondSpace = remainder.indexOf(' ');
    if (secondSpace == -1) return;
    String targetStr = remainder.substring(0, secondSpace);
    uint8_t targetID = targetStr.toInt();
    String cmd = remainder.substring(secondSpace + 1);
    sendMessage(targetID, cmd);
  }
  else if (input.startsWith("token")) {
    int firstSpace = input.indexOf(' ');
    if (firstSpace == -1) return;
    String targetStr = input.substring(firstSpace + 1);
    uint8_t nextID = targetStr.toInt();
    passToken(nextID);
  }
  else {
    Serial.println("Comando desconhecido. Use 'cmd <dest> <msg>' ou 'token <dest>'.");
  }
}
