#include "line.h"
#include "Motor_Driver.h"

void IRSensorSetup(){
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN6);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN7,GPIO_HIGH_TO_LOW_TRANSISTION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN6,GPIO_HIGH_TO_LOW_TRANSISTION);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN7);
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN6);
    Interrupt_enableInterrupt(INT_PORT1);

}

void PORT1_IRQHandler(void)
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    if(status & GPIO_PIN6)
    {
        setDirection('r');
    }
    if(status & GPIO_PIN7)
    {
        setDirection('l');
    }
    if((status & GPIO_PIN6) && (status & GPIO_PIN7))
    {
        setDirection('s');
    }

    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
}

