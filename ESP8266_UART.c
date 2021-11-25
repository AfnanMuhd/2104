#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "HCSR04.h"

volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE], ESP8266Data[ESP8266_BUFFER_SIZE], ESP8266ReceiveData[ESP8266_BUFFER_SIZE];
volatile uint32_t UARTA2ReadIndex, UARTA2ReceiveIndex = 0, index = 0, ESP8266DataIndex = 0;
volatile bool UARTA2Receive = false, ESPStartUp = false;

volatile uint8_t UARTA0Data[UARTA2_BUFFERSIZE], ID = 0;
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0ReceiveIndex = 0;
volatile bool UARTA0Receive = false, instructionFlag = false;

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

void esp8266StartUp(void)
{
    unsigned short count = 0;
    uint8_t *commands[6];
    commands[0] ="AT\r\n";
    commands[1] = "ATE0\r\n";
    commands[2] ="AT+CIPMUX=1\r\n";
    commands[3] ="AT+CIPSERVER=1,80\r\n";
    commands[4] ="AT+CWLIF\r\n";
    commands[5] = "AT+CIPSERVER=0\r\n";

    while(ESPStartUp == false)
    {
        UART_Write(commands[count]);
        __delay_cycles(24000);
        if(UARTA2Receive == true)
        {
            while((UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K') && UARTA2Data[UARTA2ReceiveIndex-7] != 'E');

            UARTA2Receive = false;
            if(UARTA2Data[UARTA2ReceiveIndex-4] == 'O' && UARTA2Data[UARTA2ReceiveIndex-3] == 'K')
            {
                while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                count++;
                if(count == 4) ESPStartUp = true;
            }
            else if(UARTA2Data[UARTA2ReceiveIndex-7] == 'E')
            {
                if(count == 2)
                {
                    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                    UART_Write(commands[4]);
                    __delay_cycles(240000);
                    if(UARTA2Data[UARTA2ReceiveIndex-4] == 'O' && UARTA2Data[UARTA2ReceiveIndex-3] == 'K')
                    {
                        UART_Write(commands[5]);
                        __delay_cycles(24000);
                    }
                    else while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                }
                else if(count == 3)
                {
                    if(UARTA2Data[UARTA2ReceiveIndex-26] == 'l') ESPStartUp = true;
                    else while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                }
            }
            else
            {
                while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
            }
        }
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
}

void ESP8266Terminal(void)
{
    uint16_t mult = 1, speed = 0, distance = 0;
    while(1)
    {
        if(UARTA2Receive == true && ESPStartUp == true && instructionFlag == true)
        {
            while(UARTA2Data[UARTA2ReceiveIndex] != ':') UARTA2ReceiveIndex--;


            UARTA2ReceiveIndex++;
            while(UARTA2Data[UARTA2ReceiveIndex] != 'e' || UARTA2Data[UARTA2ReceiveIndex+1] != 'n' || UARTA2Data[UARTA2ReceiveIndex+2] != 'd')
            {
                while(UARTA2Data[UARTA2ReceiveIndex] != '=') UARTA2ReceiveIndex++;
                UARTA2ReceiveIndex++;

                index = 0;

                while(UARTA2Data[UARTA2ReceiveIndex] != '&')
                {
                    index++;
                    UARTA2ReceiveIndex++;
                }
                UARTA2ReceiveIndex--;
                while(index != 0)
                {
                    speed += ((UARTA2Data[UARTA2ReceiveIndex] - 48) * mult);
                    mult *= 10;
                    index--;
                    UARTA2ReceiveIndex--;
                }
                mult = 1;
                UARTA2ReceiveIndex++;

                while(UARTA2Data[UARTA2ReceiveIndex] != '=') UARTA2ReceiveIndex++;
                UARTA2ReceiveIndex++;

                index = 0;

                while(UARTA2Data[UARTA2ReceiveIndex] != '&')
                {
                    index++;
                    UARTA2ReceiveIndex++;
                }
                UARTA2ReceiveIndex--;
                while(index != 0)
                {
                    distance += ((UARTA2Data[UARTA2ReceiveIndex] - 48) * mult);
                    mult *= 10;
                    index--;
                    UARTA2ReceiveIndex--;
                }
                mult = 1;
                UARTA2ReceiveIndex++;

                //ESP8266Data[index] = UARTA2Data[UARTA2ReceiveIndex];
                //index++;
                //UARTA2ReceiveIndex++;
            }
            UARTA2ReceiveIndex = 0;
        }

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

void UART_Write(uint8_t *Data)
{
    unsigned short i = 0;
    while(*(Data+i))
    {
        UART_transmitData(EUSCI_A2_BASE, *(Data+i));  // Write the character at the location specified by pointer
        i++;                                             // Increment pointer to point to the next character
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

        if(c == '+') instructionFlag = true;
        else if(c == 10 && instructionFlag != true) UARTA2Receive = true;


        if(instructionFlag == true)
        {
            if(c == '+') UARTA2ReceiveIndex = 0;
            UARTA2Data[UARTA2ReceiveIndex] = c;
            UARTA2ReceiveIndex++;
            if(c == '3' && UARTA2Data[UARTA2ReceiveIndex-2] == '2' && UARTA2Data[UARTA2ReceiveIndex-3] == '%')
            {
                UARTA2Data[UARTA2ReceiveIndex] = '\r';
                UARTA2ReceiveIndex++;
                UARTA2Data[UARTA2ReceiveIndex] = '\n';
                UARTA2ReceiveIndex++;
                UARTA2Receive = true;
            }
            else UARTA2Receive = false;
        }
        else
        {
            UARTA2Data[UARTA2ReceiveIndex] = c;
            UARTA2ReceiveIndex++;
        }

        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
    if(UARTA2Receive == true && ESPStartUp == true)
    {
        if(UARTA2Data[UARTA2ReceiveIndex-9] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'T')
        {
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
            UARTA2ReceiveIndex = 0;
            UARTA2Receive = false;
        }
        else if(UARTA2Data[UARTA2ReceiveIndex-8] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'D')
        {
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            UARTA2ReceiveIndex = 0;
            UARTA2Receive = false;
        }
    }
}
