// ================== TAREFA NO NÚCLEO 1 ==================

void core1Task() {
  uint8_t myID = devManager.getID();
  dest = (myID + 1) % TOTAL_DEVICES;
  commands[0] = "g u " + String(dest);
  commands[1] = "u " + String(dest) + " " + String(100);
  int indexToSend = 0;
  while (1) {

    String msg_rcv = com_protocol(commands[indexToSend], dest);

    if (msg_rcv != "") {
      responses[indexToSend] = msg_rcv;
      indexToSend = !indexToSend;
      //Serial.println(indexToSend);
    }
  }
}
