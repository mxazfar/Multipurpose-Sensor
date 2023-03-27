#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define RCGCI2C_R (*((volatile uint32_t *)0x400FE620))
#define RCGCGPIO_R (*((volatile uint32_t *)0x400FE608))

#define GPIOAFSELB_R (*((volatile uint32_t *)0x40005420))
#define GPIODENB_R (*((volatile uint32_t *)0x4000551C))
#define GPIOODRB_R (*((volatile uint32_t *)0x4000550C))
#define GPIODATAB_R (*((volatile uint32_t *)0x400053FC))

#define I2CMC_R (*((volatile uint32_t *)0x40020020))
#define I2CMTP_R (*((volatile uint32_t *)0x4002000C))
#define I2CMSA_R (*((volatile uint32_t *)0x40020000))
#define I2CMD_R (*((volatile uint32_t *)0x40020008))
#define I2CMCS_R (*((volatile uint32_t *)0x40020004))

int main(void) {
   SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_XTAL_4MHZ | SYSCTL_OSC_MAIN);

   RCGCI2C_R |= 0x01;
   RCGCGPIO_R |= 0x02;

   GPIOAFSELB_R |= 0x0B;
   GPIODENB_R |= 0x0B;
   GPIOODRB_R |= 0x08;
   GPIODATAB_R &= ~0x08;

   I2CMC_R |= 0x10;
   I2CMTP_R |= 0x09;
   I2CMSA_R |= 0x3F << 1;
   I2CMD_R |= 0x08;
   I2CMCS_R |= 0x07;

   uint8_t busyMask = 0x01;
   while(!(I2CMC_R & busyMask)) {

   }
}
