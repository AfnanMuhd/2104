#include "Motor_Driver.h"
#include "line.h"

extern volatile uint32_t iIndex;
extern bool isnFlag;

uint8_t state='s';
bool runFlag = false;
uint16_t forwardPWM_Left = 9000, forwardPWM_Right = 9000;

extern bool syncflag;

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        30000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        3000
};


/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig2 =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        30000,
        TIMER_A_CAPTURECOMPARE_REGISTER_4,
        TIMER_A_OUTPUTMODE_RESET_SET,
        3000
};

void MotorSetup(void)
{
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
}

void SetRightDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = forwardPWM_Left;

    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 3000;


}

void SetTurnRightDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 12000;

    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 0;


}

void SetLeftDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 3000;

    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = forwardPWM_Right;


}

void SetTurnLeftDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 0;

    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 9000;


}

void SetForwardDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 18000;


    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 18000;


}

void SetStop(void)
{
    /*left motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 0;


    /*right motor*/
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 0;


}

void SetReverseDirection(void)
{
    /*left motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    pwmConfig.dutyCycle = 27000;


    /*right motor*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
    pwmConfig2.dutyCycle = 27000;


}

/*Function to set the motor direction*/
void setDirection(char dir)
{
    state = dir;
    if(runFlag == false || dir == 's')
    {
        if(runFlag == false) runFlag = true;
        switch(dir)
        {
            case 'l':   if(isnFlag == true) SetTurnLeftDirection();
                        else SetLeftDirection();
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
                        break;

            case 'r':   if(isnFlag == true) SetTurnRightDirection();
                        else SetRightDirection();
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
                        break;

            case 'f':   SetForwardDirection();
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
                        Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
                        break;

            case 'b':   SetReverseDirection();
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
                        break;

            case 's':   SetStop();
                        syncflag = false;
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
                        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
                        break;
        }
    }
}

/*Function to set the motor speed*/
void SetMotorSpeed(double Lspeed, double Rspeed)
{
    pwmConfig.dutyCycle = Lspeed;
    pwmConfig2.dutyCycle = Rspeed;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
}

/*Function to calculate and call the setMotorSpeed function*/
void SetBaseSpeed(uint16_t lNotch, uint16_t rNotch)
{
    uint16_t leftCycle = 0, rightCycle = 0, pwmleft = 0, pwmright = 0, notches = 0, diff = 0;

    /*Calculates the pwm per notch*/
    pwmleft = pwmConfig.dutyCycle / lNotch;
    pwmright = pwmConfig2.dutyCycle / rNotch;

    /*Find the difference and sync the number of notches between the left and right motors by calculating their pwm duty cycle values*/
    if(lNotch != rNotch)
    {
        if(lNotch > rNotch)
        {
            diff = (lNotch - rNotch) / 2;
            notches = lNotch - diff;

            leftCycle = pwmConfig.dutyCycle - (diff * pwmleft);
            rightCycle = pwmConfig2.dutyCycle + (diff * pwmright);
        }
        else
        {
            diff = (rNotch - lNotch) / 2;
            notches = rNotch - diff;


            leftCycle = pwmConfig.dutyCycle + (diff * pwmleft);
            rightCycle = pwmConfig2.dutyCycle - (diff * pwmright);
        }
    }

    /*Calculates the pwm duty cycle such that both motors runs at 20 notches per second, then set the motor speed*/
    forwardPWM_Left = 20 * (leftCycle / notches);
    forwardPWM_Right = 20 * (rightCycle / notches);
    SetMotorSpeed(forwardPWM_Left, forwardPWM_Right);
}
