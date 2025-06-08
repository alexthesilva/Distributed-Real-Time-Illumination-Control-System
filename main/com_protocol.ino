String com_protocol(String command, uint8_t dest) {
  protocol.update();  // Processa mensagens CAN e atualiza a troca de token

  // Caso exista uma mensagem recebida, exibe-a no Serial Monitor
  String msg = protocol.receiveMessage();
  if (msg.length() > 0) {
    //Serial.print("Mesagem recebida:");
    if (msg.indexOf("RESP") != -1)
      return msg.substring(11);
  }

  if (protocol.hasToken()) {
    digitalWrite(LED_BUILTIN, HIGH);
    if (!commandSent) {
      // Seleciona o dispositivo destino: (meuID + 1) mod TOTAL_DEVICES

      //Serial.print("Enviando mensagem (valor HEX): ");
      //Serial.print(command);
      //Serial.print(" para dispositivo ");
      //Serial.println(dest);

      protocol.sendMessage(dest, command);
      commandSent = true;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    commandSent = false;
  }
  return "";
}
