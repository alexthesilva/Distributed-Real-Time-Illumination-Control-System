#ifndef SETUP_H
#define SETUP_H

#define BAUD_RATE 115200
#define DEVICE_ID 1 //0

// Pins Hardware
#define LED_PIN 15
#define LDR_PIN 26

// Parameters for Lamp.cpp
#define R2 10000.0       // 10k Ohm
#define VCC 3.3          // 3.3V supply
#define gammaVal 0.53    // Gamma value from LDR datasheet
#define R0 1787238.32    // Reference resistance value
#define MAX_POWER 0.108 /*Watt*/

// Parameters for Controller.cpp
#define G_USED_SETPOINT 0.49
#define FF_GAIN 1.2
#define ONE_OVER_Ti 0.02
#define D_GAIN 5
#define ANTI_WINDUP_GAIN 0.5

#define FS_CONTROLLER 100
#define SIZE_CIRCULAR_BUFFER 20


/*CAN BUS*/
#define ID_DEVICE_B1 13 // Pino para definir o bit 1 do ID do dispositivo
#define ID_DEVICE_B0 14 // Pino para definir o bit 0 do ID do dispositivo

#define TOTAL_DEVICES 3

/* CAN BUS PARAMETERS */
#define CS_PIN 17
#define CAN_BAUDRATE CAN_500KBPS
#define MCP_CLOCK MCP_8MHZ
#define CAN_ID 0x321
#define MAX_SIZE_DATA 640
#define RETRIES 5            // Tempo (em ms) para cada verificação do ACK
#define RETRANSMIT_INTERVAL 100


#endif
