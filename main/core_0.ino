// ================== VARIÁVEIS GLOBAIS ==================



String uValues[3];
String yValues[3];

// ================== LOOP PRINCIPAL ==================
String reponse_own;
void loop() {
  unsigned long currentTime = micros();  // Tempo atual em ms

  // ================== LEITURA CONTÍNUA ==================
  lamp.update_lux();
  float y = lamp.get_lux();

  // ================== EXECUÇÃO DO CONTROLADOR ==================
  if (currentTime - lastTime >= (1000000.0 / FS_CONTROLLER)) {

    // Serial.print("$");
    // Serial.print(1000000.0/(currentTime - lastTime),5);
    // Serial.println("$");

    lastTime = currentTime;
    float u;
    float reference_L = 0;

    if (controller.getManualMode()) {
      u = controller.getManualDuty();
    } else {
      reference_L = controller.getSetPointLux();
      //u = controller.integral_controller(reference_L, y);
      u = controller.pid_controller(reference_L, y);
    }

    lamp.set_duty(u);

    // Atualiza métricas para monitoramento
    metrics.update(y, u, currentTime, reference_L);
    sampleBuffer.push({ y, u });
  }
  //update_values();
  if (stringComplete) {
    int my_id = devManager.getID();
    int id_cmd = getDeviceID(inputString);
    if (id_cmd == my_id){
      reponse_own = commandprocessor.processCommand(inputString);
    }
    else if(id_cmd != -1){
      commands[1] = inputString;
      dest = id_cmd;
    }
    // Limpa o comando para a próxima leitura
    inputString = "";
    stringComplete = false;
  }
/*
    Serial.print(yValues[0]+" | ");
    Serial.print(yValues[1]+" | ");
    Serial.print(yValues[2]+" | ");
    Serial.print(uValues[0]+" | ");
    Serial.print(uValues[1]+" | ");
    Serial.println(uValues[2]);
  */
  Serial.print("Otimizador: ");
  Serial.print(responses[0]);
  Serial.print(" | Serial: ");
  Serial.print(responses[1]);
  Serial.print(" | Proprio"+String(devManager.getID())+": ");
  Serial.println(reponse_own);
}

void update_values(){
  int my_id = devManager.getID();
  for (int i = 0; i<TOTAL_DEVICES; i++){
    String cmd = "g u " + String(i);
    if (i == my_id){
      uValues[my_id] = commandprocessor.processCommand(cmd);
    }
    commands[0] = cmd;
    delay(100);
    uValues[i] = responses[0];
  }
  for (int i = 0; i<TOTAL_DEVICES; i++){
    String cmd = "g y " + String(i);
    if (i == my_id){
      yValues[my_id] = commandprocessor.processCommand(cmd);
    }
    commands[0] = cmd;
    delay(100);
    yValues[i] = responses[0];
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    if (inChar == '\n') {
      stringComplete = true;  // Comando completo
    } else if (inChar != '\r') {
      inputString += inChar;
    }
  }
}
