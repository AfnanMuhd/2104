/*******************************************************************************
 * MSP432 Timer_A - Continuous Overflow Interrupt
 *
 *  Description: Toggle P1.0 using software and the Timer0_A overflow ISR.
 *  In this example an ISR triggers when TA overflows. Inside the ISR P1.0
 *  is toggled. Toggle rate is exactly 0.5Hz. (2 second period)
 *
 *  ACLK = TACLK = 32768Hz, MCLK = SMCLK = DCO = 3MHz
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST         P1.0  |---> P1.0 LED
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 ******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#define TICKPERIOD      46875
void SetForwardDirection();
void SetSpeeds();
void slow();


/* Statics */
static volatile bool flipFlop;
uint32_t notchesdetected;
double rpm;
uint32_t notchesdetected2;
double rpm2;
double diff;
int pwmleft=0;
int pwmright=0;

/* Statics */
   /* Timer_A PWM Configuration Parameter */
    //left
   Timer_A_PWMConfig pwmConfig =
   {
           TIMER_A_CLOCKSOURCE_SMCLK,
           TIMER_A_CLOCKSOURCE_DIVIDER_24,
           10000,
           TIMER_A_CAPTURECOMPARE_REGISTER_1,
           TIMER_A_OUTPUTMODE_RESET_SET,
           3000
   };


   /* Timer_A PWM Configuration Parameter */
   //right
   Timer_A_PWMConfig pwmConfig2 =
   {
           TIMER_A_CLOCKSOURCE_SMCLK,
           TIMER_A_CLOCKSOURCE_DIVIDER_24,
           10000,
           TIMER_A_CAPTURECOMPARE_REGISTER_4,
           TIMER_A_OUTPUTMODE_RESET_SET,
           3000
   };

void Initalise_timer(void)
{
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_64,          // SMCLK/3 = 1MHz
            TICKPERIOD,                             // 1000 tick period
            TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                        // Clear value
    };

    int a = CS_getSMCLK();



    /* Configuring Timer_A0 for Up Mode */
    Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);

    /* Enabling interrupts and starting the timer */
    Interrupt_enableInterrupt(INT_TA2_0);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

    //Timer_A_stopTimer(TIMER_A0_BASE);
    Timer_A_clearTimer(TIMER_A2_BASE);

}

void WheelEncoderSetup()
{
    //wheel encoder 1
       GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);
       GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

       GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
       GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);


       //wheel encoder 2
       GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN2);

       GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN2);
       GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN2);

       Interrupt_enableInterrupt(INT_PORT5);
       Interrupt_enableMaster();
}
void MotorSetup(){

    /*left motor*/
        /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt */
        GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5);
        GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

        /*right motor*/
        /* Configuring P4.0 and P4.2 as Output. P2.5 as peripheral output for PWM and P1.4 for button interrupt */
        GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
        GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);
        GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);


}

int main(void)
{
    /* Stop watchdog timer */
    MAP_WDT_A_holdTimer();
    Initalise_timer();
    WheelEncoderSetup();
    MotorSetup();
    SetForwardDirection();
    //SetSpeeds();





    while(1)
    {
        //MAP_PCM_gotoLPM0();
    }

}

//******************************************************************************
//
//This is the TIMER_A interrupt vector service routine.
//
//******************************************************************************
void TA2_0_IRQHandler(void)
{

    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);\
    SetSpeeds();

    //printf("First wheel:%2f ",rpm);
    //printf("Second wheel:%2f\n",rpm2);
    printf("PWM Left:%d ",notchesdetected2);
    printf("PWM Right:%d\n",notchesdetected);
    notchesdetected=0;
    rpm=0;
    notchesdetected2=0;
    rpm2=0;



    /* Clear interrupt flag */
        Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);

}


/* GPIO ISR */
void PORT5_IRQHandler(void)
{
    uint32_t status;
    status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    if(status & GPIO_PIN0){
        notchesdetected++; //right
        rpm = (double)notchesdetected*60.0/20.0;
    }
    if(status & GPIO_PIN2){
         notchesdetected2++; //left
         rpm2 = (double)notchesdetected*60.0/20.0;
    }

    GPIO_clearInterruptFlag(GPIO_PORT_P5, status);
}





void SetForwardDirection(){
    /*left motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);


    /*right motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);


}
void slow(){
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 0;


    /*right motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 30000;

}
void SetLeftMotorSpeed(double speed){
    //left config2
    pwmConfig2.dutyCycle -= speed ;
    pwmConfig.dutyCycle += speed ;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
}

void SetRightMotorSpeed(double speed){
    pwmConfig2.dutyCycle += speed ;
    pwmConfig.dutyCycle -= speed ;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
}

void SetSpeeds(){

    pwmleft=pwmConfig.dutyCycle /  notchesdetected2;
    pwmright=pwmConfig2.dutyCycle /  notchesdetected;
    if(notchesdetected != notchesdetected2)
    {
    if(notchesdetected>notchesdetected2)
    {
        if(notchesdetected>notchesdetected2)//if left faster than right
        {
            diff = (notchesdetected - notchesdetected2)/2;
            SetLeftMotorSpeed(pwmConfig.dutyCycle+(diff*pwmleft));
            SetRightMotorSpeed(pwmConfig2.dutyCycle-(diff*pwmright));

        }
        /*else //if right faster than left
        {
            diff = (notchesdetected2 - notchesdetected)*10;
            SetRightMotorSpeed(diff);
        }
        */
    }
    else
    {
        if(notchesdetected>notchesdetected2)//if left faster than right
        {
            diff = (notchesdetected - notchesdetected2)/2;
            SetLeftMotorSpeed(pwmConfig.dutyCycle-(diff*pwmleft));
            SetRightMotorSpeed(pwmConfig2.dutyCycle+(diff*pwmright));

        }
    }
    }


    /*
        if (rpm != rpm2)
        {
            if(rpm>rpm2)//if left faster than right
            {
                diff = (rpm - rpm2)*10;
                SetLeftMotorSpeed(diff);

            }
            else //if right faster than left
            {
                diff = (rpm2 - rpm)*10;
                SetRightMotorSpeed(diff);
            }
        }*/
}
