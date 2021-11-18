#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define MIN_DISTANCE    10.0f
#define TICKPERIOD      1000

void HCSR04Setup(void);
float getHCSR04Distance(void);
