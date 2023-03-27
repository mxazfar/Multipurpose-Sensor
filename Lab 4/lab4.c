#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#define GPIOAFSEL_A (*((volatile uint32_t *)0x40004420))
#define GPIOPCTL_A (*((volatile uint32_t *)0x4000452C))
#define GPIODEN_A (*((volatile uint32_t *)0x4000451C))

#define GPIOAFSEL_B (*((volatile uint32_t *)0x40005420))
#define GPIOPCTL_B (*((volatile uint32_t *)0x4000552C))
#define GPIODEN_B (*((volatile uint32_t *)0x4000551C))

void transmitChunk(char* message) {
    int temp = strlen(message);
    int i;

    for(i = 0; i < temp; i++) {
        UARTCharPut(UART0_BASE, message[i]);
    }
}


int main(void) {
    char message[] = "Hello world!";

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
    {
    }

    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, 0x08);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, 0x04);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x33, false);

    GPIOAFSEL_A |= 0x03;
    GPIOPCTL_A |= 0x11;
    GPIODEN_A |= 0x03;

    I2CMasterEnable(I2C0_BASE);
    //I2CMasterDataPut(I2C0_BASE, '+');
    //I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    //while(I2CMasterBusBusy(I2C0_BASE))
    //{
   // }

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    strcpy(message, "Sensor Control\n");
    transmitChunk(message);

    strcpy(message, "Type + to turn LED on, type - to turn LED off\n");
    transmitChunk(message);

    while(1) {
        char c = UARTCharGet(UART0_BASE);

        if(c == '+') {
            strcpy(message, "Turning LED on...\n");
            transmitChunk(message);

            I2CMasterDataPut(I2C0_BASE, '+');
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
            while(I2CMasterBusBusy(I2C0_BASE))
            {
            }
        } else if(c == '-') {
            strcpy(message, "Turning LED off...\n");
            transmitChunk(message);

            I2CMasterDataPut(I2C0_BASE, '-');
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
            while(I2CMasterBusBusy(I2C0_BASE))
            {
            }
        } else {
            strcpy(message, "Unrecognized command\n");
            transmitChunk(message);
        }
    }


}


