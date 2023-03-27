#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif


//The delay was calculated based on a 20MHz clock signal and the amount of instructions required for a single iteration of a for loop.
//This function gives a delay of 0.25 seconds, and can be used to construct the 0.75 and 3 second delays.
void delay() {
   volatile uint32_t ui32Loop;

   for(ui32Loop = 0; ui32Loop < 40000; ui32Loop++)
   {
   }
}

//Takes in the color as an integer--0 for red, 1 for blue.
void blink(uint8_t color, uint8_t numTimes) {
    volatile uint8_t ui8Loop;

    if(color == 0) {
        for(ui8Loop = 0; ui8Loop < numTimes; ui8Loop++) {
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
           delay();

           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
           //Three delays results in a total delay of 0.75 seconds
           delay();
           delay();
           delay();
        }
    } else if(color == 1) {
        for(ui8Loop = 0; ui8Loop < numTimes; ui8Loop++) {
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
           delay();

           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
           delay();
           delay();
           delay();
       }
    }
}

int
main(void)
{
    volatile uint8_t delayCounter;
    volatile uint8_t color = 0; //0 for red, 1 for blue.

    //Sets clock speed to 20 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10);

    //Supply clock signal to Port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    //Set GPIO Pin 1 (Red) to output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    //Set GPIO Pin 2 (Blue) to output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    while(1) {
       blink(color, 3);

       //12 delays results in a delay of 3 seconds
       for(delayCounter = 0; delayCounter < 12; delayCounter++) {
           delay();
       }
    }
}
