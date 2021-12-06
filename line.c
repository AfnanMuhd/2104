#include "line.h"
#include "Motor_Driver.h"

char lineDir = '0';
uint8_t sensorState = '0';

void IRSensorSetup(){
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN6);
    /*
    GPIO_interruptEdgeSelect(GPIO_PORT_P6,GPIO_PIN7,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P6,GPIO_PIN6,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(GPIO_PORT_P6,GPIO_PIN7);
    GPIO_clearInterruptFlag(GPIO_PORT_P6,GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P6,GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P6,GPIO_PIN6);
    Interrupt_enableInterrupt(INT_PORT6);
    */

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

//void PORT6_IRQHandler(void)
//{

    //uint32_t status;
    //status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P6);
    /*if(status & GPIO_PIN6)
    {
        lineDir = 'r';
        if(sensorState == '0')
        {
            sensorState = 'l';
            lineDir = 'r';
        }
        else if(sensorState == 'r')
        {
            sensorState = 's';
            lineDir = 's';
        }
        else
        {
            sensorState = '0';
            //lineDir = 'l';
        }
    }
    else if(status & GPIO_PIN7)
    {
        lineDir = 'l';
        if(sensorState == '0')
        {
            sensorState = 'r';
            lineDir = 'l';
        }
        /*else if(sensorState == 'l')
        {
            sensorState = 's';
            lineDir = 's';
        }
        else
        {
            sensorState = '0';
            //lineDir = 'r';
        }

    }

    if((status & GPIO_PIN6) && (status & GPIO_PIN7))
    {
        setDirection('s');
    }
*/
   // GPIO_clearInterruptFlag(GPIO_PORT_P6, status);
//}

