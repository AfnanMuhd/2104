/* Standard Includes */
#include "PID.h"

/* Statics */
uint16_t notchesdetected, notchesdetected2, timer_count = 0;

bool syncflag = false, speedflag = false;
extern uint8_t state;

/* Statics */
   //Timer_A PWM Configuration Parameter
   //left
   /*Timer_A_PWMConfig pwmConfig_1 =
   {
       TIMER_A_CLOCKSOURCE_SMCLK,
       TIMER_A_CLOCKSOURCE_DIVIDER_24,
       10000,
       TIMER_A_CAPTURECOMPARE_REGISTER_1,
       TIMER_A_OUTPUTMODE_RESET_SET,
       4000
   };


   // Timer_A PWM Configuration Parameter
   //right
   Timer_A_PWMConfig pwmConfig_2 =
   {
       TIMER_A_CLOCKSOURCE_SMCLK,
       TIMER_A_CLOCKSOURCE_DIVIDER_24,
       10000,
       TIMER_A_CAPTURECOMPARE_REGISTER_4,
       TIMER_A_OUTPUTMODE_RESET_SET,
       4000
   };*/

void Initalise_encoderTimer(void)
{
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig encoderUpConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_64,          // SMCLK/3 = 1MHz
            TICKPERIOD2,                             // 1000 tick period
            TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                        // Clear value
    };

    int a = CS_getSMCLK();

    /* Configuring Timer_A0 for Up Mode */
    Timer_A_configureUpMode(TIMER_A2_BASE, &encoderUpConfig);

    /* Enabling interrupts and starting the timer */
    Interrupt_enableInterrupt(INT_TA2_0);
    Timer_A_clearTimer(TIMER_A2_BASE);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

    //Timer_A_stopTimer(TIMER_A0_BASE);
    //Timer_A_clearTimer(TIMER_A2_BASE);

}

void WheelEncoderSetup(void)
{
    //wheel encoder 1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);

    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);

    //wheel encoder 2
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);

    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);

    Interrupt_enableInterrupt(INT_PORT5);
    //Interrupt_enableMaster();
}
/*void MotorSetup1(void)
{
    //left motor
    // Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

    //right motor
    // Configuring P4.0 and P4.2 as Output. P2.5 as peripheral output for PWM and P1.4 for button interrupt
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig_1);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig_2);

}*/

//******************************************************************************
//
//This is the TIMER_A interrupt vector service routine.
//
//******************************************************************************

void TA2_0_IRQHandler(void)
{
    if(timer_count != 6) timer_count++;
    else
    {
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
        if(syncflag == false && state == 'f')
        {
            syncflag = true;
            speedflag = false;
            SetSpeeds(notchesdetected, notchesdetected2);
        }
        else if(syncflag == true && speedflag == false)
        {
            speedflag = true;
            if((notchesdetected<19 || notchesdetected>21) && (notchesdetected2<19 || notchesdetected2>21))
                SetBaseSpeed(notchesdetected, notchesdetected2);
        }
        //printf("PWM Left:%d ", notchesdetected);
        //printf("PWM Right:%d\n", notchesdetected2);
        notchesdetected=0;
        notchesdetected2=0;
        timer_count = 0;


    }
    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    //printf("First wheel:%2f ",rpm);
    //printf("Second wheel:%2f\n",rpm2);

    //printf("PWM Left:%d ", notchesdetected);
    //printf("PWM Right:%d\n", notchesdetected2);
}

/* GPIO ISR */
void PORT5_IRQHandler(void)
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    if(status & GPIO_PIN0)
    {
        notchesdetected++; //right
        //rpm = (double)notchesdetected*60.0/20.0;
    }
    if(status & GPIO_PIN2)
    {
         notchesdetected2++; //left
         //rpm2 = (double)notchesdetected*60.0/20.0;
    }

    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);
}
