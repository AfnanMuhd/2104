#include "ESP8266_UART.h"
#include "Motor_Driver.h"
#include "PID.h"
#include "line.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>

void main()
{
    MAP_WDT_A_holdTimer();

    /*Ensure MSP432 is Running at 24 MHz*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);

    /*Setup LED pins*/
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    /*Ultrasonic sensor setup*/
    HCSR04Setup();

    /*UART module setup for ESP8266*/
    UARTStartUp();

    /*Motor driver setup*/
    MotorSetup();

    /*IR Optical speed sensor setup*/
    Initalise_encoderTimer();
    WheelEncoderSetup();

    //IRSensorSetup();

    /*Reset GPIO of the ESP8266*/
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1);

    /*Enable interrupt*/
    MAP_Interrupt_enableMaster();

    /*Startup for ESP8266*/
    esp8266StartUp();

    /*Enter ESP8266 terminal*/
    ESP8266Terminal();
}
