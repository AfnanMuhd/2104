#include <stdint.h>
#include <stdbool.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

void MotorSetup(void);
void SetRightDirection(void);
void SetLeftDirection(void);
void SetForwardDirection(void);
void setDirection(char);
void SetSpeeds(uint16_t lNotch, uint16_t rNotch);
void SetMotorSpeed(double Lspeed, double Rspeed);
