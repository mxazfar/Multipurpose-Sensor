#include <msp430.h>

/*
 * Moosa Azfar, Gilbert Traczyk
 * 10/2/2022
 * This program toggles between showing the raw ADC output and gravity scaled readings
 * from a three axis accelerometer.
 */

//Pin mapping
//GFEDCBA.
//76543210
//4321
//7654

///////////////////////////////////////////////////////////////////////////////////////////
// Displays gravity scale reading in requested format based on raw reading
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void displayGravityScale(int coordinate, int adcReading);

///////////////////////////////////////////////////////////////////////////////////////////
// Select ADC channel based on current coordinate
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void adcChannelSelect(int coordinate);

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in current reading, next reading and returns stable reading
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
int removeOscillations(int currentReading, int nextReading);

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a coordinate (0-2) as input and displays DP accordingly. An input of 0 shows the X coordinate, 1 shows Y, etc.
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void displayDP(int coordinate);

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a character as input and displays the character on a seven segment hex display
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void seg7Display(int input);

//////////////////////////////////////////////////////////////////////////////////////////////
// Displays input onto Quad-7Segment Display by flashing individual segments on and off
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void displayQuad7Seg(int adcReading);

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes ports and ADC
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void setup();

//Global interrupt/state flags
int timerFlag = 0;
int buttonFlag = 0;
int coordinateFlag = 0;

//Calls setup, then goes into infinite loop that executes different functions based on state
void main(void)
{
    setup();

    int adcReading = ADC10MEM;
    int next = adcReading;

    while(1) {
        if(buttonFlag == 0) {
            displayQuad7Seg(adcReading);
            displayDP(coordinateFlag);

            next = ADC10MEM;
            adcReading = removeOscillations(adcReading, next);

            adcChannelSelect(coordinateFlag);
        } else if(buttonFlag == 1) {
            displayGravityScale(coordinateFlag, adcReading);
            displayDP(2);

            next = ADC10MEM;
            adcReading = removeOscillations(adcReading, next);
            adcChannelSelect(coordinateFlag);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes ports and ADC
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR &= ~0x0F; //Set P1 bits 0-3 as input
    P1DIR |= 0xF0; // Set P1 bits 4-7 as output
    P1IE |= BIT3; //Enable interrupts on bit 3
    P1IES |= BIT3; //Trigger interrupt on falling edge
    P1REN |= BIT3; //Enable pullup resistor on P1.3
    P1IFG &= ~BIT3; //Clear P1.3 interrupt flag

    P2SEL &= ~0xC0; //Set P2 bits 6 and 7 to digital output instead of XIN/XOUT
    P2DIR |= 0xFF; //Set entire P2 to output;
    P2OUT |= 0xFF; //Ensure all P2 bits are off

    P1SEL |= BIT0 + BIT1 + BIT2; //Set BIT0-2 of P1 as analog input
    ADC10AE0 = 0x07; //Select analog channel A0-2
    ADC10CTL1 = INCH_0 + ADC10DIV_3; //Set ADC clock to clock division 3, default to input channel 0
    ADC10CTL0 = ADC10SHT_3 + MSC + ADC10ON; //Turn ADC ON
    ADC10CTL0 |= ENC + ADC10SC; //Enable ADC

    CCTL0 = CCIE; //Enable interrupts on timer
    CCR0 = 50000; //Load timer with initial value
    TACTL = TASSEL_2 + MC_2; //Set timer to use system clock, count continuously
    __enable_interrupt(); //Enable interrupts globally
}

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a character as input and displays the character on a seven segment hex display
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void seg7Display(int input) {
    P2OUT |= 0xFE;

    switch(input) {
        case 0:
            P2OUT &= ~(0x3F << 1);
            break;
        case 1:
            P2OUT &= ~(0x06 << 1);
            break;
        case 2:
            P2OUT &= ~(0x5B << 1);
            break;
        case 3:
            P2OUT &= ~(0x4F << 1);
            break;
        case 4:
            P2OUT &= ~(0x66 << 1);
            break;
        case 5:
            P2OUT &= ~(0x6D << 1);
            break;
        case 6:
            P2OUT &= ~(0x7D << 1);
            break;
        case 7:
            P2OUT &= ~(0x07 << 1);
            break;
        case 8:
            P2OUT &= ~(0x7F << 1);
            break;
        case 9:
            P2OUT &= ~(0x6F << 1);
            break;
        case 10:
            P2OUT &= ~(0x77 << 1);
            break;
        case 11:
            P2OUT &= ~(0x7C << 1);
            break;
        case 12:
            P2OUT &= ~(0x39 << 1);
            break;
        case 13:
            P2OUT &= ~(0x5E << 1);
            break;
        case 14:
            P2OUT &= ~(0x79 << 1);
            break;
        case 15:
            P2OUT &= ~(0x71 << 1);
            break;
        default:
            P2OUT = 0xFE;
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////
// Displays ADC reading onto 4-bar 7 segment LED display by flashing individual segments on and off based on ADC reading.
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////
void displayQuad7Seg(int adcReading) {
    volatile int i;

    P2OUT |= 0xFE;

    P1OUT &= ~0xF0;
    P1OUT |= 0x80;
    seg7Display(adcReading % 10);

    int adcReadingTemp = adcReading / 10;

    for(i = 0; i < 250; i++);

    if(adcReading > 9) {
        P2OUT |= 0xFE;

        P1OUT &= ~0x80;
        P1OUT |= 0x40;
        seg7Display(adcReadingTemp % 10);
        adcReadingTemp /= 10;

        for(i = 0; i < 250; i++);
    }

    if(adcReading > 99) {
        P2OUT |= 0xFE;

        P1OUT &= ~0x40;
        P1OUT |= 0x20;
        seg7Display(adcReadingTemp % 10);


        for(i = 0; i < 250; i++);
    }

    if(adcReading > 999) {
        P2OUT |= 0xFE;

        P1OUT &= ~0x20;
        P1OUT |= 0x10;
        seg7Display(1);

        for(i = 0; i < 250; i++);
    }

    P2OUT |= 0xFE;
    P1OUT &= ~0xF0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a coordinate (0-2) as input and displays DP accordingly. An input of 0 shows the X coordinate, 1 shows Y, etc.
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void displayDP(int coordinate) {
    volatile int i;
    P2OUT |= 0x01; //Initialize decimal to off
    P1OUT &= ~0xF0; //Initialize all displays to off

    //Switch based on coordinate, flash corresponding DP on and off
    switch(coordinate) {
        case 0:
            P1OUT |= 0x10;
            P2OUT &= ~0x01;

            for(i = 0; i < 250; i++);
            P2OUT |= 0x01;
            break;
        case 1:
            P1OUT &= ~0x10;
            P1OUT |= 0x20;
            P2OUT &= ~0x01;

            for(i = 0; i < 250; i++);
            P2OUT |= 0x01;
            break;
        case 2:
            P1OUT &= ~0x20;
            P1OUT |= 0x40;
            P2OUT &= ~0x01;

            for(i = 0; i < 250; i++);
            P2OUT |= 0x01;
            break;
    }

    P1OUT &= ~0xF0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in current reading, next reading and returns stable reading
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
int removeOscillations(int currentReading, int nextReading) {

    //If the next reading is +- 4 of the current reading, increment the current reading
    if(nextReading >= currentReading + 4) {
        currentReading += 1;
    } else if(nextReading <= currentReading - 4) {
        currentReading -= 1;
    }

    //If the next reading is +- 5 of the current reading, set current reading to next reading.
    //This is done to address sharp changes in readings.
    if(nextReading >= currentReading + 5) {
        currentReading = nextReading;
    } else if(nextReading <= currentReading - 5) {
        currentReading = nextReading;
    }

    //If next reading is at either ends, set the next reading to either end.
    if(nextReading == 1023 || nextReading == 0) {
        currentReading = nextReading;
    }

    return currentReading;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Select ADC channel based on current coordinate
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void adcChannelSelect(int coordinate) {
    ADC10CTL0 &= ~(ENC + ADC10SC); //Disable ADC to change control registers

    switch(coordinateFlag) {
        case 0:
            ADC10CTL1 = INCH_0;
            break;
        case 1:
            ADC10CTL1 = INCH_1;
            break;
        case 2:
            ADC10CTL1 = INCH_2;
            break;
    }

    ADC10CTL0 |= ENC + ADC10SC; //Enable ADC
}

///////////////////////////////////////////////////////////////////////////////////////////
// Displays gravity scale reading in requested format based on raw reading
// @mazfar@uncc.edu + gtraczyk@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void displayGravityScale(int coordinate, int adcReading) {
    volatile int i, gravityScale, gravityScaleTemp;
    P1OUT |= 0x10; // Turn off first (coordinate) segment

    // Display X/Y/Z based on coordinate by flashing on and off
    switch(coordinate) {
    case 0:
        P2OUT &= ~(0x76 << 1);
        break;
    case 1:
        P2OUT &= ~(0x6E << 1);
        break;
    case 2:
        seg7Display(2);
        break;
    }

    //Hold segment on
    for(i = 0; i < 250; i++);

    //Turn segment off, clear inputs
    P1OUT &= ~0x10;
    P2OUT |= 0xFE;

    //Displays negative sign if needed by flashing on/off
    P1OUT |= 0x20;

    if(adcReading < 494) {
        P2OUT &= ~(0x40 << 1);
    }

    for(i = 0; i < 250; i++);

    P1OUT &= ~0x20;
    P2OUT |= 0xFE;

    //Convert adcReading to gravity scale (-30 to 30)
    if(coordinate == 2){
        gravityScale = ((adcReading-204)*10)/86 - 32;//changes 0 by each axis bias
    }else if(coordinate == 0){
        gravityScale = ((adcReading-204)*10)/86 - 32;//changes 0 by each axis bias
    }else{
        gravityScale = ((adcReading-204)*10)/86 - 30;//changes 0 by each axis bias
    }




    //Since we've already dealt with sign, take absolute value of gravityScale
    if(gravityScale < 0) {
        gravityScale *= -1;
    }

    //Display gravityScale via flashing individual displays on and off, see displayQuad7Seg for more details
    P1OUT |= 0x80;
    seg7Display(gravityScale % 10);

    gravityScaleTemp = gravityScale / 10;

    for(i = 0; i < 250; i++);

    if(gravityScale > 9) {
        P2OUT |= 0xFE;

        P1OUT &= ~0x80;
        P1OUT |= 0x40;
        seg7Display(gravityScaleTemp % 10);

        for(i = 0; i < 250; i++);
    } else {
        P2OUT |= 0xFE;

        P1OUT &= ~0x80;
        P1OUT |= 0x40;
        seg7Display(0);

        for(i = 0; i < 250; i++);
    }

    P1OUT &= ~0xF0;
    P2OUT |= 0xFE;
}

///////////////////////////////////////////////////////////////////////////////////////////
// ISR for Timer. Updates coordinate flag after 60 iterations, resulting in delay of 3 seconds.
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
    //Checks if timer flag has iterated 60 times. If so, reset timer flag and advance coordinate flag.
    if(timerFlag == 59) {
        timerFlag = 0;
        if(coordinateFlag == 2) {
            coordinateFlag = 0;
        } else {
            coordinateFlag++;
        }
    } else {
        timerFlag++;
    }

    //Reloads timer with initial value;
    CCR0 += 50000;
}

///////////////////////////////////////////////////////////////////////////////////////////
// ISR for Button. Updates button flag on interrupt
// @gtraczyk@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
//    __disable_interrupt(); //disables interrupts
    int i;
    for(i=0;i<2000;i++); // pauses for ~.4 seconds to debounce
    if((P1IN & BIT3) == 0){ //only Toggle if button is pressed after debounce period
        if(buttonFlag == 0) {//Toggle buttonFlag value between 0 and 1
            buttonFlag = 1;
        } else if(buttonFlag == 1) {
            buttonFlag = 0;
        }
    }
//    __enable_interrupt();
    P1IFG &= ~BIT3; //Clear interrupt flag
}
