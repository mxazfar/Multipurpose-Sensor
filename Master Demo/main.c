#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

//PB0, PB1
//PD0-3

int colorArray[21];
int inputArray[21];
int currentColor;
int numLevel;
int inputNum;
int timeoutCount;
int blinkDelay;
bool correctInput;

#define STRELOAD (*((volatile uint32_t *)0xE000E014))
#define STCURRENT (*((volatile uint32_t *)0xE000E018))
#define STCTRL (*((volatile uint32_t *)0xE000E010))
#define INT_EN0_R (*((volatile uint32_t *)0xE000E100))
#define INT_DIS0_R (*((volatile uint32_t *)0xE000E180))
#define GPIOIM_R (*((volatile uint32_t *)0x40007410))
#define GPIOMIS_R (*((volatile uint32_t *)0x40007418))
#define GPIOIC_R (*((volatile uint32_t *)0x4000741C))
#define GPIOIS_R (*((volatile uint32_t *)0x40007404))

//0 = red, 1 = green, 2 = blue

void gpioHandler(void) {
    INT_DIS0_R |= 0x08;

    for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

    }

    if(GPIOMIS_R & 0x01) {
        inputNum++;
        inputArray[inputNum] = 0;
        GPIOIC_R |= 0x07;
    } else if(GPIOMIS_R & 0x02) {
        inputNum++;
        inputArray[inputNum] = 1;
        GPIOIC_R |= 0x07;
    } else if(GPIOMIS_R & 0x04) {
        inputNum++;
        inputArray[inputNum] = 2;
        GPIOIC_R |= 0x07;
    }

    INT_EN0_R |= 0x08;
}

void systickHandler(void) {
    timeoutCount++;
}

void blinkLed(uint8_t color) {
    switch(color) {
        case 0:
            GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }

            GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0x0);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }
            break;

        case 1:
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }

            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x0);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }
            break;

        case 2:
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }

            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x0);
            for(blinkDelay = 0; blinkDelay < 100000; blinkDelay++) {

            }
            break;
    }
}


int main(void) {
    uint8_t loopInt = 0;
    uint32_t loopInt1 = 0;
    srand(time(NULL));
    currentColor = rand();
    numLevel = 0;
    timeoutCount = 0;
    correctInput = false;

    //Configure clock speed to 20MHz and a GPIO+Interrupt stuff
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_XTAL_4MHZ |SYSCTL_OSC_MAIN);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 |GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
    INT_EN0_R |= 0x08;
    GPIOIM_R |= 0x07;

    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
    //Clear color array
    for(loopInt = 0; loopInt < 19; loopInt++) {
        colorArray[loopInt] = -1;
    }

    while(1) {
        //Calculates new blink randomly or resets array
        if(numLevel <= 20) {
            currentColor = rand() % 3;
            colorArray[numLevel] = currentColor;
        } else {
            numLevel = 0;
            for(loopInt = 0; loopInt < 19; loopInt++) {
                    colorArray[loopInt] = -1;
            }

            currentColor = rand() % 3;
            colorArray[numLevel] = currentColor;
        }

        //Blinks new pattern
        for(loopInt = 0; loopInt <= numLevel; loopInt++) {
            blinkLed(colorArray[loopInt]);
        }

        //Clear input array
        for(loopInt = 0; loopInt < 19; loopInt++) {
            inputArray[loopInt] = -1;
        }

        inputNum = -1;

        STRELOAD |= 0xBF0000;
        STCURRENT |= 0x01;
        STCTRL |= 0x07;

        //Systick counts down while waiting for interaction from GPIO
        while(timeoutCount < 2) {
            //Leaves while loop if total expected inputs have been recieved.
            if(inputNum == numLevel) {
                break;
            }
        }
        //At this point, you've either lost or entered the right stuff, need to check both. Regardless, systick needs to be disabled so it stops pulling us out.
        STCTRL &= ~0x07;
        //If timeoutCount reaches 2, you waited for 15 seconds and didn't say anything, so you lost.
        if(timeoutCount == 2) {
            for(loopInt = 0; loopInt < 3; loopInt++) {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
                for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                }

                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
                for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                }
            }

            numLevel = 21;
        } else { //If we enter this branch, we know we got the expected amount of inputs, still need to check if they're correct.
            for(loopInt = 0; loopInt <= numLevel; loopInt++) {
                //If statement checks each element. If even one is found as false, we break, if not, run through the entire thing.
                if(inputArray[loopInt] == colorArray[loopInt]) {
                    correctInput = true;
                } else {
                    correctInput = false;
                    break;
                }
            }

            if(correctInput) {
                numLevel++;

                for(loopInt = 0; loopInt < 3; loopInt++) {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                    for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                    }

                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
                    for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                    }
                }
            } else {
                numLevel = 21;

                for(loopInt = 0; loopInt < 3; loopInt++) {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);

                    for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                    }

                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);
                    for(loopInt1 = 0; loopInt1 < 100000; loopInt1++) {

                    }
                }
            }
        }

        timeoutCount = 0;
    }
}
