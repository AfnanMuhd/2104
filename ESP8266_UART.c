#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "HCSR04.h"

volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE], ESP8266Data[ESP8266_BUFFER_SIZE], ESP8266ReceiveData[ESP8266_BUFFER_SIZE], Token[6] = {'a', '1', 'b', '2', 'c', '3'};
volatile uint32_t UARTA2ReadIndex, UARTA2ReceiveIndex = 0, index = 0, ESP8266DataIndex = 0;
volatile bool UARTA2Receive = false, ESPStartUp = false, connFlag = false;

volatile uint8_t UARTA0Data[UARTA2_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex, UARTA0ReceiveIndex = 0;
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
    uint8_t *commands[7];
    commands[0] ="AT\r\n";
    commands[1] = "ATE0\r\n";
    commands[2] ="AT+CIPMUX=1\r\n";
    commands[3] ="AT+CIPSERVER=1,80\r\n";
    commands[4] ="AT+CIPSTO=5\r\n";
    commands[5] ="AT+CWLIF\r\n";
    commands[6] = "AT+CIPSERVER=0\r\n";

    while(ESPStartUp == false)
    {
        UART_Write(commands[count]);
        __delay_cycles(24000);
        if(UARTA2Receive == true)
        {
            while((UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K') && UARTA2Data[UARTA2ReceiveIndex-7] != 'E');

            UARTA2Receive = false;
            if(UARTA2Data[UARTA2ReceiveIndex-7] == 'E')
            {
                if(count == 2)
                {
                    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                    UART_Write(commands[5]);
                    __delay_cycles(240000);
                    if(UARTA2Data[UARTA2ReceiveIndex-4] == 'O' && UARTA2Data[UARTA2ReceiveIndex-3] == 'K')
                    {
                        UART_Write(commands[6]);
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
                count++;
                if(count == 5) ESPStartUp = true;
            }
        }
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
}

void setToken(void)
{
    while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
    ESP8266DataIndex++;

    index = 0;
    while(index != 6)
    {
        Token[index] = ESP8266Data[ESP8266DataIndex];
        index++;
        ESP8266DataIndex++;
    }
    ESP8266DataIndex = 0;
}

void POST(uint8_t ID)
{
    uint16_t mult = 1, speed = 0, distance = 0, parts = 0, part_cnt = 0, temp_data, temp_data_cnt = ESP8266DataIndex;
    uint16_t data[2] = {0, 0};
    uint8_t tok[6];

    if(ESP8266Data[ESP8266DataIndex-3] != '3'&& ESP8266Data[ESP8266DataIndex-4] != '2' && ESP8266Data[ESP8266DataIndex-5] != '%')
    {
        while(ESP8266DataIndex > 0) ESP8266Data[--ESP8266DataIndex] = 0x00;
    }
    else
    {
        while(ESP8266Data[ESP8266DataIndex] != ':')
        {
            if(ESP8266Data[ESP8266DataIndex] == '&') part_cnt++;
            ESP8266DataIndex--;
        }
        part_cnt -= 2;
        ESP8266DataIndex += 5;

        if(ESP8266Data[ESP8266DataIndex] == '0' && ESP8266Data[ESP8266DataIndex+1] == '&') setToken();
        else if(ESP8266Data[ESP8266DataIndex] == '1' && ESP8266Data[ESP8266DataIndex+1] == '&')
        {
            while(parts < part_cnt)
            {
                index = 0;
                while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
                ESP8266DataIndex++;

                while(ESP8266Data[ESP8266DataIndex] != '&')
                {
                    index++;
                    ESP8266DataIndex++;
                }
                ESP8266DataIndex--;

                while(index != 0)
                {
                    data[parts] += ((ESP8266Data[ESP8266DataIndex] - 48) * mult);
                    mult *= 10;
                    index--;
                    ESP8266DataIndex--;
                }
                mult = 1;
                parts++;
                ESP8266DataIndex++;
            }

            index = 0;
            while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
            ESP8266DataIndex++;

            while(index != 6)
            {
                if(Token[index] == ESP8266Data[ESP8266DataIndex])
                {
                    index++;
                    ESP8266DataIndex++;
                }
                else break;
            }
            ESP8266DataIndex = 0;

            if(index!= 6)
            {
                ESP8266DataIndex = temp_data_cnt;
                while(ESP8266DataIndex > 0) ESP8266Data[--ESP8266DataIndex] = 0x00;
            }
            else
            {
                instructionFlag = false;
                sendSuccess(ID);
                while(temp_data_cnt > 0) ESP8266Data[--temp_data_cnt] = 0x00;
            }
        }
        else
        {
            UART_Write("AT+CIPCLOSE=0\r\n");
        }
    }
}

void sendSuccess(uint8_t ID)
{
    bool sendFlag = false;
    uint16_t index = 0;
    while(!sendFlag)
    {
        UART_Write("AT+CIPSENDEX=0,2048\r\n");
        __delay_cycles(24000);
        if(UARTA2Receive == true)
        {
            while((UARTA2Data[UARTA2ReceiveIndex-2] != '>' || UARTA2Data[UARTA2ReceiveIndex-1] != ' ') && UARTA2Data[UARTA2ReceiveIndex-7] != 'E');

            UARTA2Receive = false;
            if(UARTA2Data[UARTA2ReceiveIndex-2] == '>' && UARTA2Data[UARTA2ReceiveIndex-1] == ' ')
            {
                while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                while(!sendFlag)
                {
                    UART_Write("HTTP/1.1 200 OK\\0");
                    __delay_cycles(24000);
                    if(UARTA2ReceiveIndex > 0)
                    {
                        while(index < UARTA2ReceiveIndex-4)
                        {
                            if(UARTA2Data[index] == 'S' && UARTA2Data[index+1] == 'E' && UARTA2Data[index+2] == 'N' && UARTA2Data[index+3] == 'D' && UARTA2Data[index+4] == ' ' && UARTA2Data[index+5] == 'O' && UARTA2Data[index+6] == 'K') break;
                            else index++;
                        }
                        if(index != UARTA2ReceiveIndex-4)
                        {
                            sendFlag = true;
                            while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
                        }
                    }
                }
            }
            else while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
        }
    }
}

void GET(uint8_t ID, uint8_t type)
{
    while(UARTA2Data[UARTA2ReceiveIndex-2] != '>' || UARTA2Data[UARTA2ReceiveIndex-1] != ' ')
    {
        UART_Write("AT+CIPSENDEX=0,27\r\n");
        __delay_cycles(24000);
    }

    UART_Write("HTTP/1.1 200 OK\n\na1b2c3\\0");
    while(UARTA2Data[UARTA2ReceiveIndex-9] != 'S' || UARTA2Data[UARTA2ReceiveIndex-8] != 'E' || UARTA2Data[UARTA2ReceiveIndex-7] != 'N' || UARTA2Data[UARTA2ReceiveIndex-6] != 'D' || UARTA2Data[UARTA2ReceiveIndex-5] != ' ' || UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K')
    {
        //UART_Write("HTTP/1.1 200 OK\n\na1b2c3\\0");
        //UART_Write("AT+CIPSEND=0,27\r\n");
        __delay_cycles(24000);
    }

    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
}

void ESP8266Terminal(void)
{
    uint16_t index;
    uint8_t ID;
    while(1)
    {
        if(instructionFlag == true)
        {
            __delay_cycles(240000);
            if(UARTA2Receive == true && ESPStartUp == true && instructionFlag == true)
            {
                ID = ESP8266Data[5];
                index = 0;
                while(index < ESP8266DataIndex)
                {
                    if(ESP8266Data[index] == 'P' && ESP8266Data[index+1] == 'O' && ESP8266Data[index+2] == 'S' && ESP8266Data[index+3] == 'T')
                    {
                        POST(ID);
                        break;
                    }
                    else if(ESP8266Data[index] == 'G' && ESP8266Data[index+1] == 'E' && ESP8266Data[index+2] == 'T')
                    {
                        while(index < ESP8266DataIndex && ESP8266Data[index] != '=') index++;
                        index++;
                        GET(ID, ESP8266Data[index]);
                        instructionFlag = false;
                        break;
                    }
                    else index++;
                }
            }
        }

        /*if((getHCSR04Distance() < MIN_DISTANCE))
        {
            setDirection('s');
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            while((getHCSR04Distance() < MIN_DISTANCE));
        }
        else
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);*/
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

        if(c == '+' && instructionFlag != true)
        {
            instructionFlag = true;
            UARTA2ReceiveIndex = 0;
        }
        else if(c == 10 && instructionFlag != true) UARTA2Receive = true;

        UARTA2Data[UARTA2ReceiveIndex] = c;
        UARTA2ReceiveIndex++;

        if(instructionFlag == true)
        {
            //if(c == '+') UARTA2ReceiveIndex = 0;
            //UARTA2Data[UARTA2ReceiveIndex] = c;
            //UARTA2ReceiveIndex++;
            if((c == '3' && UARTA2Data[UARTA2ReceiveIndex-2] == '2' && UARTA2Data[UARTA2ReceiveIndex-3] == '%') || (UARTA2Data[UARTA2ReceiveIndex-1] == '\n' && UARTA2Data[UARTA2ReceiveIndex-2] == '\r' && UARTA2Data[UARTA2ReceiveIndex-3] == '\n' && UARTA2Data[UARTA2ReceiveIndex-4] == '\r' && UARTA2Data[UARTA2ReceiveIndex-5] == 'e'))
            {
                ESP8266DataIndex = UARTA2ReceiveIndex;
                while(UARTA2ReceiveIndex > 0)
                {
                    ESP8266Data[UARTA2ReceiveIndex] = UARTA2Data[UARTA2ReceiveIndex];
                    UARTA2Data[UARTA2ReceiveIndex] = 0x00;
                    UARTA2ReceiveIndex--;
                }
                ESP8266Data[UARTA2ReceiveIndex] = UARTA2Data[UARTA2ReceiveIndex];
                UARTA2Data[UARTA2ReceiveIndex] = 0x00;
                ESP8266Data[ESP8266DataIndex] = '\r';
                ESP8266DataIndex++;
                ESP8266Data[ESP8266DataIndex] = '\n';
                ESP8266DataIndex++;
                UARTA2Receive = true;
            }
            else UARTA2Receive = false;
        }

        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
    if(UARTA2Receive == true && ESPStartUp == true)
    {
        if(UARTA2Data[UARTA2ReceiveIndex-9] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'T')
        {
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
            connFlag = true;
            UARTA2Receive = false;
            //UARTA2ReceiveIndex = 0;
            //UARTA2Receive = false;
        }
        else if(UARTA2Data[UARTA2ReceiveIndex-8] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'D')
        {
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            connFlag = false;
            while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
            connFlag = false;
            UARTA2Receive = false;
            //UARTA2ReceiveIndex = 0;
            //UARTA2Receive = false;
        }
    }
}
