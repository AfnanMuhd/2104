#include "HCSR04.h"
#include "Motor_Driver.h"

volatile uint32_t SR04IntTimes;

void HCSR04Setup(void)
{
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_24,          // SMCLK/3 = 1MHz
            TICKPERIOD,                             // 1000 tick period
            TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                        // Clear value
    };

    int a = CS_getSMCLK();

    /* Configuring P3.6 as Output */
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);//
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);//

    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P3, GPIO_PIN7);


    /* Configuring Timer_A0 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /* Enabling interrupts and starting the timer */
    Interrupt_enableInterrupt(INT_TA1_0);
    //Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //Timer_A_stopTimer(TIMER_A0_BASE);
    Timer_A_clearTimer(TIMER_A1_BASE);

}

float getHCSR04Distance(void)
{
    uint32_t pulsetime=0;
    float calculateddistance = 0;

    /* Generate 10us pulse at P3.6 */
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
    __delay_cycles(240);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);

    /* Wait for positive-edge */
    while(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN7) == 0);

    /* Start Timer */
    SR04IntTimes = 0;
    Timer_A_clearTimer(TIMER_A1_BASE);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    /* Detects negative-edge */
    while(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN7) == 1);

    /* Stop Timer */
    Timer_A_stopTimer(TIMER_A1_BASE);

    /* Obtain Pulse Width in microseconds */
    /* Number of times the interrupt occurred (1 interrupt = 1000 ticks)    */
    pulsetime = SR04IntTimes * TICKPERIOD;

    /* Number of ticks (between 1 to 999) before the interrupt could occur */
    pulsetime += Timer_A_getCounterValue(TIMER_A1_BASE);

    /* Clear Timer */
    Timer_A_clearTimer(TIMER_A1_BASE);

    //__delay_cycles(3000);

    /* Calculating distance in cm */
    calculateddistance = (float)pulsetime / 58.0f;

    return calculateddistance;
}

void TA1_0_IRQHandler(void)
{
    /* Increment global variable (count number of interrupt occurred) */
    SR04IntTimes++;

    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
