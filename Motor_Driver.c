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

/*Timer_A_UpModeConfig motorUpConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_64,          // SMCLK/3 = 1MHz
        62500,                                  // 1000 tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};*/

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

    //setDirection('f');
    /* Configuring Timer_A0 for Up Mode */
    //Timer_A_configureUpMode(TIMER_A3_BASE, &motorUpConfig);

    /* Enabling interrupts and starting the timer */
    //Interrupt_enableInterrupt(INT_TA3_0);
    //Timer_A_clearTimer(TIMER_A3_BASE);
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

void SetMotorSpeed(double Lspeed, double Rspeed)
{
    pwmConfig.dutyCycle = Lspeed;
    pwmConfig2.dutyCycle = Rspeed;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
}

/*void SetSpeeds(uint16_t lNotch, uint16_t rNotch)
{
    uint16_t leftCycle = 0, rightCycle = 0, diff = 0, pwmleft = 0, pwmright = 0;
    pwmleft = pwmConfig.dutyCycle /  lNotch;
    pwmright = pwmConfig2.dutyCycle /  rNotch;

    if(lNotch != rNotch)
    {
        if(lNotch > rNotch)
        {
            diff = (lNotch - rNotch) / 2;

            leftCycle = pwmConfig.dutyCycle - (diff * pwmleft);
            rightCycle = pwmConfig2.dutyCycle + (diff * pwmright);

            SetMotorSpeed(leftCycle, rightCycle);
        }
        else
        {
            diff = (rNotch - lNotch) / 2;

            leftCycle = pwmConfig.dutyCycle + (diff * pwmleft);
            rightCycle = pwmConfig2.dutyCycle - (diff * pwmright);

            SetMotorSpeed(leftCycle, rightCycle);
        }
    }
}*/

void SetBaseSpeed(uint16_t lNotch, uint16_t rNotch)
{
    uint16_t leftCycle = 0, rightCycle = 0, pwmleft = 0, pwmright = 0, notches = 0, diff = 0;
    pwmleft = pwmConfig.dutyCycle / lNotch;
    pwmright = pwmConfig2.dutyCycle / rNotch;

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

    forwardPWM_Left = 20 * (leftCycle / notches);
    forwardPWM_Right = 20 * (rightCycle / notches);
    SetMotorSpeed(forwardPWM_Left, forwardPWM_Right);
}
