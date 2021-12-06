#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "HCSR04.h"
extern char lineDir;
extern bool runFlag;

volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE], ESP8266Data[ESP8266_BUFFER_SIZE], ESP8266ReceiveData[ESP8266_BUFFER_SIZE], instruction_coms[3] = { NULL }, Token[50] = { NULL };
volatile uint32_t UARTA2ReadIndex, UARTA2ReceiveIndex = 0, index = 0, ESP8266DataIndex = 0, tokIndex = 0, part_cnt = 0, iIndex = 0, instruct_index = 0;
volatile bool UARTA2Receive = false, ESPStartUp = false, connFlag = false, isnFlag = false;

volatile uint8_t UARTA0Data[UARTA2_BUFFERSIZE], dir = 'f', senstat = '0', data[2048] = { NULL };
volatile uint32_t UARTA0ReadIndex, UARTA0ReceiveIndex = 0;
volatile bool UARTA0Receive = false, instructionFlag = false;

void UARTStartUp(void)
{
    /*UART configuration for ESP8266*/
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

    /*Initialize required hardware peripherals for the ESP8266*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A2_BASE, &UART2Config);
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
}

/*Function for setting up the ESP8266 WiFi module*/
void esp8266StartUp(void)
{
    unsigned short count = 0, retries;
    uint8_t *commands[9];

    /*Commands to test and set up ESP8266*/
    commands[0] ="AT\r\n";
    commands[1] = "ATE0\r\n";
    commands[2] ="AT+CWMODE=2\r\n";
    commands[3] ="AT+CWSAP=\"ROBOTIC_CAR2\",\"1234567890\",5,3\r\n";
    commands[4] ="AT+CIPAP=\"192.168.4.1\"\r\n";
    commands[5] ="AT+CIPSERVER=0\r\n";
    commands[6] ="AT+CIPMUX=1\r\n";
    commands[7] ="AT+CIPSERVER=1,80\r\n";
    commands[8] ="AT+CIPSTO=1\r\n";

    /*While loop to ensure that the ESP8266 successfully start up*/
    while(ESPStartUp == false)
    {
        retries = 10;

        /*function call to send the commands to ESP8266 via UART*/
        UART_Write(commands[count]);
        __delay_cycles(24000);

        while((UARTA2Data[UARTA2ReceiveIndex-4] != 'O' || UARTA2Data[UARTA2ReceiveIndex-3] != 'K') && UARTA2Data[UARTA2ReceiveIndex-7] != 'E')
        {
            __delay_cycles(24000);
            if(--retries == 0) break;
        }

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

    /*Clear the UART2Data array and output the appropriate LED after successful set up*/
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
}

/*Function for running the robotic car after successful set up of ESP8266*/
void ESP8266Terminal(void)
{
    uint16_t index;

    /*Infinite while loop that contains codes for running the robotic car*/
    while(1)
    {
        /*if statement for parsing POST and GET requests from received data*/
        if(instructionFlag == true)
        {
            __delay_cycles(240000);
            if(UARTA2Receive == true && ESPStartUp == true && instructionFlag == true)
            {
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
            }
            instructionFlag = false;
        }

        /*if statement to run instructions received*/
        if(part_cnt != 0)
        {
            if(iIndex != instruct_index)
            {
                if(data[iIndex] == 'l' || data[iIndex] == 'r')
                {
                    if(GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN7) && GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN6) && isnFlag == true)
                    {
                        data[iIndex] = '0';
                        setDirection('s');
                        isnFlag = false;
                        iIndex++;
                    }
                    else isnFlag = true;
                }
                setDirection(data[iIndex]);
            }
        }

        /*if statement to stop the robotic car movement using the ultrasonic sensor*/
        if((getHCSR04Distance() < MIN_DISTANCE))
        {
            setDirection('s');
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            while((getHCSR04Distance() < MIN_DISTANCE));
        }
        else GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

        /*readLine();
        if(senstat != lineDir && isnFlag == false)
        {
            senstat = lineDir;
            setDirection(lineDir);
        }*/
    }
}

/*This function is for determining the validity and type of data received via POST method*/
void POST(void)
{
    uint16_t postIndex = ESP8266DataIndex;
    if(ESP8266Data[ESP8266DataIndex-5] == '%' && ESP8266Data[ESP8266DataIndex-4] == '2' && ESP8266Data[ESP8266DataIndex-3] == '3')
    {
        while(ESP8266Data[ESP8266DataIndex-3] != 'I' || ESP8266Data[ESP8266DataIndex-2] != 'S' || ESP8266Data[ESP8266DataIndex-1] != 'N' || ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex--;

        /*if a 0 is received, signifies a set token instruction*/
        if(ESP8266Data[ESP8266DataIndex+1] == '0' && ESP8266Data[ESP8266DataIndex+2] == '&')
        {
            ESP8266DataIndex++;
            setToken();
        }

        /*if a 1 is received, signifies a motion instruction*/
        else if(ESP8266Data[ESP8266DataIndex+1] == '1' && ESP8266Data[ESP8266DataIndex+2] == '&')
        {
            ESP8266DataIndex = postIndex;
            instruction();
        }

        /*Sends a success message back to the robotic car once the data has been parsed*/
        sendSuccess();
    }
}

/*This function is for processing a GET request*/
void GET(uint8_t type)
{
    uint16_t getIndex = 0;
    char header[ESP8266_BUFFER_SIZE];

    /* Crafting message to be send*/
    strcpy(header,  "HTTP/1.1 200 OK\n\n");

    index = 17;
    while(index != (tokIndex+17))
    {
        header[index] = Token[index-17];
        index++;
    }

    index = 0;

    /*Command to initialize data transmission*/
    UART_Write("AT+CIPSENDEX=0,39\r\n");
    __delay_cycles(2400000);
    while(UARTA2Data[UARTA2ReceiveIndex-1] != '>')
    {
        UART_Write("AT+CIPSENDEX=0,39\r\n");
        __delay_cycles(2400000);
        getIndex = UARTA2ReceiveIndex;
    }

    /*Sending message crafted above*/
    UART_Write(header);
    __delay_cycles(24000);
    getIndex = UARTA2ReceiveIndex;
    while(UARTA2Data[getIndex-3] != 'R' || UARTA2Data[getIndex-2] != 'e' || UARTA2Data[getIndex-1] != 'c' || UARTA2Data[getIndex] != 'v' )
    {
        if(getIndex != 0) getIndex--;
        else
        {
            UART_Write(header);
            __delay_cycles(24000);
            getIndex = UARTA2ReceiveIndex;
        }
    }

    /*Clear the UART2Data array after successful transmission*/
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
}

/*Function to set token from received data*/
void setToken(void)
{
    /*Traverse to the character '=' in the received data*/
    while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
    ESP8266DataIndex++;

    tokIndex = 0;

    /*Store all characters till the '&' character as the token*/
    while(ESP8266Data[ESP8266DataIndex] != '&')
    {
        Token[tokIndex] = ESP8266Data[ESP8266DataIndex];
        tokIndex++;
        ESP8266DataIndex++;
    }
    ESP8266DataIndex = 0;
}

/*Function to parse the instructions contained in the received data*/
void instruction(void)
{
    uint16_t temp_data_cnt = ESP8266DataIndex, parts = 0, i;
    instruct_index = 0;
    part_cnt = 0;
    iIndex = 0;
    runFlag = false;

    /*Traverse to the substring "ISN" in the received data, count the number of instruction segments as well and increment the variable part_cnt at each segment*/
    while(ESP8266Data[ESP8266DataIndex] != 'I' || ESP8266Data[ESP8266DataIndex+1] != 'S' || ESP8266Data[ESP8266DataIndex+2] != 'N' || ESP8266Data[ESP8266DataIndex+3] != '=')
    {
        if(ESP8266Data[ESP8266DataIndex] == '&') part_cnt++;
        ESP8266DataIndex--;
    }
    part_cnt -= 2;
    ESP8266DataIndex += 6;

    /*Stores the instructions segments based on the part_cnt variable*/
    while(parts < part_cnt)
    {
        while(ESP8266Data[ESP8266DataIndex] != '=') ESP8266DataIndex++;
        ESP8266DataIndex++;

        dir = ESP8266Data[ESP8266DataIndex];

        if(dir == 'F' || dir == 'L' || dir == 'R' || dir == 'B')
        {
            ESP8266DataIndex++;
            for(i = 0; i < (int)(ESP8266Data[ESP8266DataIndex]-30); i++)
            {
                data[instruct_index] = dir + 20;
                instruct_index++;
            }
        }
        else
        {
            data[instruct_index] = dir;
            instruct_index++;
        }
        senstat = ESP8266Data[ESP8266DataIndex];

        parts++;
    }

    /*Clears the ESP8266Data array*/
    while(temp_data_cnt > 0) ESP8266Data[--temp_data_cnt] = 0x00;
}

/*Function for sending success message*/
void sendSuccess(void)
{
    index = 0;

    /*Command to initialize data transmission*/
    UART_Write("AT+CIPSEND=0,15\r\n");
    __delay_cycles(240000);
    while(UARTA2Data[UARTA2ReceiveIndex-1] != '>')
    {
        UART_Write("AT+CIPSENDEX=0,15\r\n");
        __delay_cycles(240000);
    }
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;

    /*message to be sent*/
    UART_Write("HTTP/1.1 200 OK");
    __delay_cycles(24000);
    while(UARTA2Data[UARTA2ReceiveIndex-15] != 'R' || UARTA2Data[UARTA2ReceiveIndex-14] != 'e' || UARTA2Data[UARTA2ReceiveIndex-13] != 'c' || UARTA2Data[UARTA2ReceiveIndex-12] != 'v');
    while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
}

/*Function to write to ESP8266 via UART*/
void UART_Write(uint8_t *Data)
{
    unsigned short i = 0;
    while(*(Data+i))
    {
        UART_transmitData(EUSCI_A2_BASE, *(Data+i));  // Write the character at the location specified by pointer
        i++;                                             // Increment pointer to point to the next character
    }
}

/*UART ISR for ESP8266*/
void EUSCIA2_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        c = MAP_UART_receiveData(EUSCI_A2_BASE);

        /*if statement to check if an instruction data is being received*/
        if(c == '+' && instructionFlag != true)
        {
            instructionFlag = true;
            while(UARTA2ReceiveIndex > 0) UARTA2Data[--UARTA2ReceiveIndex] = 0x00;
        }

        UARTA2Data[UARTA2ReceiveIndex] = c;
        UARTA2ReceiveIndex++;

        /*if statements to check if end of an instruction data has been received*/
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
    }

    /*if statements for changing LED based on connection*/
    if(UARTA2Data[UARTA2ReceiveIndex-9] == 'C' && UARTA2Data[UARTA2ReceiveIndex-3] == 'T' && UARTA2Data[UARTA2ReceiveIndex-2] == '\r')
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
