/*
  CommandParser.ino

  Este sketch demonstra como processar comandos recebidos via Serial
  extraindo o identificador do dispositivo (<i>) de acordo com os diferentes
  formatos dos comandos definidos no projeto.

  Exemplo de comandos:
    - "u <i> <val>"      (set duty cycle)
    - "g u <i>"          (get duty cycle)
    - "r <i> <val>"      (set illuminance reference)
    - "g r <i>"          (get illuminance reference)
    - "g y <i>"          (measure actual illuminance)
    - "g v <i>"          (measure voltage at LDR)
    - "o <i> <val>"      (set occupancy state)
    - "g o <i>"          (get occupancy state)
    - "a <i> <val>"      (set anti-windup state)
    - "g a <i>"          (get anti-windup state)
    - "f <i> <val>"      (set feedback control state)
    - "g f <i>"          (get feedback control state)
    - "g d <i>"          (get external illuminance)
    - "g p <i>"          (get instantaneous power)
    - "g t <i>"          (get elapsed time)
    - "s <x> <i> <val> <time>"   (start real-time stream; <x> pode ser 'y' ou 'u')
    - "S <x> <i>"        (stop real-time stream)
    - "g b <x> <i>"      (get last minute buffer; <x> pode ser 'y' ou 'u')
    - Outros comandos, conforme a especificação.
*/

#include <Arduino.h>

// Função auxiliar para retornar o token (parte) da string, considerando espaços como separadores.
// O índice é 0-baseado.
String getToken(String data, int index) {
  data.trim();
  size_t start = 0;  // Changed to size_t
  int tokenCount = 0;
  
  while (start < data.length()) {
    int spaceIndex = data.indexOf(' ', start);
    if (spaceIndex == -1) {
      if (tokenCount == index) {
        return data.substring(start);
      } else {
        return "";
      }
    }
    if (tokenCount == index) {
      return data.substring(start, static_cast<size_t>(spaceIndex)); // Handle cast
    }
    tokenCount++;
    start = static_cast<size_t>(spaceIndex) + 1; // Safe cast
    // Skip spaces
    while (start < data.length() && data.charAt(start) == ' ') {
      start++;
    }
  }
  return "";
}

// Função que, dado um comando recebido via Serial, extrai o <i> (identificador do dispositivo)
// que pode estar em posições diferentes dependendo do formato do comando.
// Retorna -1 se o comando não possuir um ID válido.
int getDeviceID(String command) {
  command.trim();
  if (command.length() == 0) return -1;
  
  // Token 0: comando principal
  String token0 = getToken(command, 0);
  
  // Comandos sem identificador, como o reset "R"
  if (token0 == "R") {
    return -1;
  }
  
  // Comandos que começam com "g" (get)
  if (token0 == "g") {
    String token1 = getToken(command, 1);
    // Comando do tipo "g b <x> <i>" (buffer) tem 4 tokens: o <i> é o token de índice 3
    if (token1 == "b") {
      String devStr = getToken(command, 3);
      return devStr.toInt();
    } else {
      // Comandos do tipo "g u", "g r", "g y", "g v", "g o", "g a", "g f", "g d", "g p", "g t", etc.
      // o <i> é o token de índice 2
      String devStr = getToken(command, 2);
      return devStr.toInt();
    }
  }
  // Comandos de streaming: "s" ou "S"
  else if (token0 == "s" || token0 == "S") {
    // Exemplo: "s <x> <i> <val> <time>" ou "S <x> <i>"
    // O <i> está no token de índice 2
    String devStr = getToken(command, 2);
    return devStr.toInt();
  }
  // Para os demais comandos (ex.: "u", "r", "o", "a", "f", "O", "U", "C", etc),
  // o <i> é considerado como o token de índice 1.
  else {
    String devStr = getToken(command, 1);
    return devStr.toInt();
  }
}
