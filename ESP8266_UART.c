#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "HCSR04.h"

volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE], ESP8266Data[ESP8266_BUFFER_SIZE], ESP8266ReceiveData[ESP8266_BUFFER_SIZE];
volatile uint32_t UARTA2ReadIndex, UARTA2ReceiveIndex = 0, index = 0, ESP8266DataIndex = 0;
volatile bool UARTA2Receive = false, ESPStartUp = false;

volatile uint8_t UARTA0Data[UARTA2_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0ReceiveIndex = 0;
volatile bool UARTA0Receive = false;

void esp8266StartUp(void)
{
    UART_Write(EUSCI_A2_BASE, "ATE0\r\n", 6);
    while(UARTA2ReceiveIndex > 0)
        UARTA2Data[--UARTA2ReceiveIndex] = 0x00;

    UART_Write(EUSCI_A2_BASE, "AT\r\n", 4);
    while(UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K')
    {
        if(UARTA2Data[UARTA2ReceiveIndex-7] == 'E' || UARTA2ReceiveIndex == 0)
        {
            UART_Write(EUSCI_A2_BASE, "AT\r\n", 4);
        }
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;

    UART_Write(EUSCI_A2_BASE, "AT+CIPMUX=1\r\n", 13);
    while(UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K')
    {
        if(UARTA2Data[UARTA2ReceiveIndex-7] == 'E')
        {
            UART_Write(EUSCI_A2_BASE, "AT+CWLIF\r\n", 11);
            if(UARTA2Data[UARTA2ReceiveIndex-4] != 'O' && UARTA2Data[UARTA2ReceiveIndex-3] != 'K') UART_Write(EUSCI_A2_BASE, "AT+CIPMUX=1\r\n", 13);
        }
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;

    UART_Write(EUSCI_A2_BASE, "AT+CIPSERVER=1,80\r\n", 19);
    while(UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K')
    {
        if(UARTA2Data[UARTA2ReceiveIndex-7] == 'E')
        {
            if(UARTA2Data[UARTA2ReceiveIndex-26] == 'l')
            {
                UARTA2Data[UARTA2ReceiveIndex-4] = 'O';
                UARTA2Data[UARTA2ReceiveIndex-3] = 'K';
                ESPStartUp = true;
            }
            else UART_Write(EUSCI_A2_BASE, "AT+CIPSERVER=1,80\r\n", 13);
        }
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    if(ESPStartUp == false) MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
    else MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
    ESPStartUp = true;
}

void ESP8266Terminal(void)
{
    while(1)
    {
        if((getHCSR04Distance() < MIN_DISTANCE))
        {
            setDirection('s');
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            while((getHCSR04Distance() < MIN_DISTANCE));
        }
        else
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

void UART_Write(uint32_t UART, uint8_t *Data, uint32_t Size)
{
    unsigned short i = 0;
    while(i <= Size)
    {
        UART_transmitData(UART, *(Data+i));  // Write the character at the location specified by pointer
        i++;                                 // Increment pointer to point to the next character
    }
}

void EUSCIA0_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        c = MAP_UART_receiveData(EUSCI_A0_BASE);
        if(c == 8)
        {
            UARTA0ReceiveIndex--;
            UARTA0Data[UARTA0ReceiveIndex] = 0;

        }
        else if(c == '\r')
        {
            UARTA0Data[UARTA0ReceiveIndex++] = '\r';
            UARTA0Data[UARTA0ReceiveIndex++] = '\n';
            MAP_UART_transmitData(EUSCI_A0_BASE, '\r');
            MAP_UART_transmitData(EUSCI_A0_BASE, '\n');
            UARTA0Receive = true;
        }
        else
        {
            UARTA0Data[UARTA0ReceiveIndex] = c;
            UARTA0ReceiveIndex++;
        }

        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
}

void EUSCIA2_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        c = MAP_UART_receiveData(EUSCI_A2_BASE);

        if(c == 10) UARTA2Receive = true;

        UARTA2Data[UARTA2ReceiveIndex] = c;
        UARTA2ReceiveIndex++;

        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }

    if(UARTA2Receive == true && ESPStartUp == true)
    {
        if(UARTA2Data[UARTA2ReceiveIndex-9] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'T')
        {
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }
        else if(UARTA2Data[UARTA2ReceiveIndex-8] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'D')
        {
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }
        if(UARTA2Data[0] == '+' && UARTA2Data[1] == 'I' && UARTA2Data[2] == 'P' && UARTA2Data[3] == 'D')
        {
            index = 4;
            while(UARTA2Data[index] != ':') index++;
            index++;

            while(UARTA2Data[index] != '\r')
            {
                ESP8266Data[ESP8266DataIndex] = UARTA2Data[index];
                index++;
            }

            //UART_Write(EUSCI_A0_BASE, &ESP8266Data, ESP8266DataIndex);
            if(ESP8266Data[0] == 'A') MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            else if(ESP8266Data[0] == 'a') MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

            if(ESP8266Data[0] == 'B') setDirection('l');

            if(ESP8266Data[0] == 'C') setDirection('r');

            if(ESP8266Data[0] == 'D') setDirection('f');
            else if(ESP8266Data[0] == 'd') setDirection('b');

            if(ESP8266Data[0] == 'E') setDirection('s');
        }
        UARTA2ReceiveIndex = 0;
        index = 0;

        UARTA2Receive = false;
    }
}

void UARTStartUp(void)
{
    eUSCI_UART_ConfigV1 UART0Config =
    {
         EUSCI_A_UART_CLOCKSOURCE_SMCLK,
         13,
         0,
         37,
         EUSCI_A_UART_NO_PARITY,
         EUSCI_A_UART_LSB_FIRST,
         EUSCI_A_UART_ONE_STOP_BIT,
         EUSCI_A_UART_MODE,
         EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
    };

    eUSCI_UART_ConfigV1 UART2Config =
    {
         EUSCI_A_UART_CLOCKSOURCE_SMCLK,
         13,
         0,
         37,
         EUSCI_A_UART_NO_PARITY,
         EUSCI_A_UART_LSB_FIRST,
         EUSCI_A_UART_ONE_STOP_BIT,
         EUSCI_A_UART_MODE,
         EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
    };

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A0_BASE, &UART0Config);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);

    /*Initialize required hardware peripherals for the ESP8266*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A2_BASE, &UART2Config);
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
}
