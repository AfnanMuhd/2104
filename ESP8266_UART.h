#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define ESP8266_BUFFER_SIZE     2048
#define UARTA_BUFFERSIZE        2048
#define UARTA2_BUFFERSIZE       2048

#define TCP                     0
#define UDP                     1

void esp8266StartUp(void);
void ESP8266Terminal(void);
void UARTStartUp(void);
void UART_Write(uint8_t *Data);
void POST(void);
void GET(uint8_t type);
void setToken(void);
void instruction(void);
void sendSuccess(void);
