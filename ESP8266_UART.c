#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "HCSR04.h"

volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE], ESP8266Data[ESP8266_BUFFER_SIZE], ESP8266ReceiveData[ESP8266_BUFFER_SIZE], instruction_coms[3] = { NULL }, Token[50] = { NULL };
volatile uint32_t UARTA2ReadIndex, UARTA2ReceiveIndex = 0, index = 0, ESP8266DataIndex = 0, tokIndex = 0;
volatile bool UARTA2Receive = false, ESPStartUp = false, connFlag = false;

volatile uint8_t UARTA0Data[UARTA2_BUFFERSIZE], dir = 'NULL';
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
    uint8_t *commands[9];
    commands[0] ="AT\r\n";
    commands[1] = "ATE0\r\n";
    commands[2] ="AT+CWMODE=2\r\n";
    commands[3] ="AT+CWSAP=\"ROBOTIC_CAR2\",\"1234567890\",5,3\r\n";
    commands[4] ="AT+CIPAP=\"192.168.4.1\"\r\n";
    commands[5] ="AT+CIPSERVER=0\r\n";
    commands[6] ="AT+CIPMUX=1\r\n";
    commands[7] ="AT+CIPSERVER=1,80\r\n";
    commands[8] ="AT+CIPSTO=5\r\n";

    while(ESPStartUp == false)
    {
        UART_Write(commands[count]);
        __delay_cycles(24000);

        while((UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K') && UARTA2Data[UARTA2ReceiveIndex-7] != 'E');

        if(UARTA2Data[UARTA2ReceiveIndex-4] == 'O' && UARTA2Data[UARTA2ReceiveIndex-3] == 'K')
        {
            count++;
            if(count == 9) ESPStartUp = true;
        }
        else
        {
            if(count == 5) count++;
        }

        while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
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
                        POST();
                        break;
                    }
                    else if(ESP8266Data[index] == 'G' && ESP8266Data[index+1] == 'E' && ESP8266Data[index+2] == 'T')
                    {
                        while(index < ESP8266DataIndex && ESP8266Data[index] != '=') index++;
                        index++;
                        GET(ESP8266Data[index]);
                        break;
                    }
                    else index++;
                }
                instructionFlag = false;
            }
        }
        if((getHCSR04Distance() < MIN_DISTANCE))
        {
            setDirection('s');
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            while((getHCSR04Distance() < MIN_DISTANCE));
        }
        else GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

void POST(void)
{
    uint16_t postIndex = ESP8266DataIndex;
    if(ESP8266Data[ESP8266DataIndex-5] == '%' && ESP8266Data[ESP8266DataIndex-4] == '2' && ESP8266Data[ESP8266DataIndex-3] == '3')
    {
        while(ESP8266Data[ESP8266DataIndex-3] != 'I' || ESP8266Data[ESP8266DataIndex-2] != 'S' || ESP8266Data[ESP8266DataIndex-1] != 'N' || ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex--;

        if(ESP8266Data[ESP8266DataIndex+1] == '0' && ESP8266Data[ESP8266DataIndex+2] == '&')
        {
            ESP8266DataIndex++;
            setToken();
        }
        else if(ESP8266Data[ESP8266DataIndex+1] == '1' && ESP8266Data[ESP8266DataIndex+2] == '&')
        {
            ESP8266DataIndex = postIndex;
            instruction();
        }
        sendSuccess();
    }
}

void GET(uint8_t type)
{
    char header[ESP8266_BUFFER_SIZE];

    strcpy(header,  "HTTP/1.1 200 OK\n\n");

    index = 17;
    while(index != (tokIndex+17))
    {
        header[index] = Token[index-17];
        index++;
    }

    index = 0;

    UART_Write("AT+CIPSENDEX=0,39\r\n");
    __delay_cycles(24000);
    while(UARTA2Data[UARTA2ReceiveIndex-2] != '>' || UARTA2Data[UARTA2ReceiveIndex-1] != ' ');

    UART_Write(header);
    __delay_cycles(24000);
    while(UARTA2Data[UARTA2ReceiveIndex-9] != 'S' || UARTA2Data[UARTA2ReceiveIndex-8] != 'E' || UARTA2Data[UARTA2ReceiveIndex-7] != 'N' || UARTA2Data[UARTA2ReceiveIndex-6] != 'D' || UARTA2Data[UARTA2ReceiveIndex-5] != ' ' || UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K');

    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
}

void setToken(void)
{
    while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
    ESP8266DataIndex++;

    tokIndex = 0;
    while(ESP8266Data[ESP8266DataIndex] != '&')
    {
        Token[tokIndex] = ESP8266Data[ESP8266DataIndex];
        tokIndex++;
        ESP8266DataIndex++;
    }
    ESP8266DataIndex = 0;
}

void instruction(void)
{
    uint16_t temp_data_cnt = ESP8266DataIndex, parts = 0, part_cnt = 0, data[2] = {0, 0};

    while(ESP8266Data[ESP8266DataIndex] != ':')
    {
        if(ESP8266Data[ESP8266DataIndex] == '&') part_cnt++;
        ESP8266DataIndex--;
    }
    part_cnt -= 2;
    ESP8266DataIndex += 6;

    while(parts < part_cnt)
    {
        while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
        ESP8266DataIndex++;
        setDirection(ESP8266Data[ESP8266DataIndex]);
        dir = ESP8266Data[ESP8266DataIndex];

        parts++;
    }

    while(temp_data_cnt > 0) ESP8266Data[--temp_data_cnt] = 0x00;
}

void sendSuccess(uint8_t ID)
{
    bool sendFlag = false;

    index = 0;

    UART_Write("AT+CIPSENDEX=0,15\r\n");
    __delay_cycles(24000);
    while(UARTA2Data[UARTA2ReceiveIndex-2] != '>' || UARTA2Data[UARTA2ReceiveIndex-1] != ' ');
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;

    UART_Write("HTTP/1.1 200 OK\\0");
    __delay_cycles(24000);
    while(UARTA2Data[UARTA2ReceiveIndex-9] != 'S' || UARTA2Data[UARTA2ReceiveIndex-8] != 'E' || UARTA2Data[UARTA2ReceiveIndex-7] != 'N' || UARTA2Data[UARTA2ReceiveIndex-6] != 'D' || UARTA2Data[UARTA2ReceiveIndex-5] != ' ' || UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K');
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
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
            while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
        }

        UARTA2Data[UARTA2ReceiveIndex] = c;
        UARTA2ReceiveIndex++;

        if(instructionFlag == true)
        {
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
        }

        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
    if(UARTA2Data[UARTA2ReceiveIndex-9] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'T')
    {
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

        while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    }
    else if(UARTA2Data[UARTA2ReceiveIndex-8] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'D')
    {
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

        while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    }
}
