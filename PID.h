#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "Motor_Driver.h"

//#define TICKPERIOD      46875
#define TICKPERIOD      62500

void Initalise_encoderTimer(void);
void WheelEncoderSetup(void);
