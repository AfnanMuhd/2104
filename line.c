#include "line.h"
#include "Motor_Driver.h"

char lineDir = '0';
uint8_t sensorState = '0';

void IRSensorSetup(){
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P6, GPIO_PIN6);
    //GPIO_interruptEdgeSelect(GPIO_PORT_P6,GPIO_PIN7,GPIO_HIGH_TO_LOW_TRANSITION);
    //GPIO_interruptEdgeSelect(GPIO_PORT_P6,GPIO_PIN6,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(GPIO_PORT_P6,GPIO_PIN7);
    GPIO_clearInterruptFlag(GPIO_PORT_P6,GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P6,GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P6,GPIO_PIN6);
    Interrupt_enableInterrupt(INT_PORT6);

}

void PORT6_IRQHandler(void)
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P6);
    if(status & GPIO_PIN6)
    {
        if(sensorState == '0')
        {
            sensorState = 'l';
            lineDir = 'r';
        }
        else
        {
            sensorState = '0';
            lineDir = 'f';
        }
    }
    else if(status & GPIO_PIN7)
    {
        if(sensorState == '0')
        {
            sensorState = 'r';
            lineDir = 'l';
        }
        else
        {
            sensorState = '0';
            lineDir = 'f';
        }

    }
    /*
    if((status & GPIO_PIN6) && (status & GPIO_PIN7))
    {
        setDirection('s');
    }
*/
    GPIO_clearInterruptFlag(GPIO_PORT_P6, status);
}

