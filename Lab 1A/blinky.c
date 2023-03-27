#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#define INT_EN0_R (*((volatile uint32_t *)0xE000E100))
#define GPIOIM_R (*((volatile uint32_t *)0x40025410))
#define GPIOICR_R (*((volatile uint32_t *)0x4002541C))

volatile uint8_t color; //0 for red, 1 for blue.
volatile uint32_t studentIDOriginal;
volatile uint8_t blinkNum;
volatile uint32_t studentID;

//#pragma INTERRUPT (interruptHandler)
void interruptHandler(void) {
   GPIOICR_R = 0x10;

   if(color == 2) {
       color = 0;
   } else {
       color++;
   }

   if(studentID / 10 != 0) {
       blinkNum = studentID % 10;
       studentID = studentID / 10;
   } else {
       blinkNum = studentID;
       studentID = studentIDOriginal;
   }
}

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

    if(color == 1) {
        for(ui8Loop = 0; ui8Loop < numTimes; ui8Loop++) {
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
           delay();

           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
           //Three delays results in a total delay of 0.75 seconds
           delay();
           delay();
           delay();
        }
    } else if(color == 0) {
        for(ui8Loop = 0; ui8Loop < numTimes; ui8Loop++) {
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
           delay();

           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
           delay();
           delay();
           delay();
       }
    } else if(color == 2) {
        for(ui8Loop = 0; ui8Loop < numTimes; ui8Loop++) {
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
           delay();

           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
           delay();
           delay();
           delay();
       }
    }
}

int main(void)
{
    volatile uint8_t delayCounter;
    volatile uint8_t temp;

    color = 2; //0 for red, 1 for blue, 2 for green.
    studentIDOriginal = 801147694;
    blinkNum = studentIDOriginal % 10;
    studentID = studentIDOriginal / 10;

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

    //Set GPIO Pin 3 (Green) to output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    //Set GPIO Pin 0 (SW_1) to input, with weak pull
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    INT_EN0_R = 0x40000000;
    GPIOIM_R |= 0x10;

    while(1) {
       blink(color, blinkNum);

       //12 delays results in a delay of 3 seconds
       for(delayCounter = 0; delayCounter < 12; delayCounter++) {
           delay();
       }
    }
}
