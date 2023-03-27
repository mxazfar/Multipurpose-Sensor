#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

int main(void)
{
    volatile uint32_t loopNum;

    SysCltPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	return 0;
}
