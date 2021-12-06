#include "line.h"
#include "Motor_Driver.h"

char lineDir = '0';
uint8_t sensorState = '0';

void IRSensorSetup()
{
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN6);
}

void readLine(void)
{
    //from left to right. Sensor output is high when white space is detected. Since we're seeking a black line,inputs are inverted
    bool sensorRight = !GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN7);//right
    bool sensorLeft = !GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN6);//left
    if(sensorLeft && sensorRight)
    {
        lineDir = 's';
    }
    else if(sensorLeft)
    {
        lineDir = 'r';
    }
    else if(sensorRight)
    {
        lineDir = 'l';
    }
    else if((!sensorLeft && lineDir == 'r') || (!sensorRight && lineDir == 'l'))
    {
        lineDir='f';
    }
}
