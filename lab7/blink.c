#include <msp430.h>

//GFEDCBAX
//76543210

//4321
//3210

int removeOscillations(int adcReading);
void seg7Display(int input);
void displayQuad7Seg(int adcReading);
void setup();

//Initializes ports and ADC, polls for changes in potentiometer and updates display accordingly
void main(void)
{
    setup();

    //int adcReading = ADC10MEM;
    //int next = ADC10MEM;

    while(1) {
        displayQuad7Seg(50);
        //adcReading = removeOscillations(adcReading);


        //ADC10CTL0 |= ENC + ADC10SC; //Enable ADC
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes ports and ADC
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= 0xF0; // Set P1 bits 4-7 as output
    P2SEL &= ~0xC0; //Set P2 bits 6 and 7 to digital output instead of XIN/XOUT
    P2DIR |= 0xFF; //Set entire P2 to output;
    P2OUT |= 0xFF; //Ensure all P2 bits are off

    P1SEL |= BIT0; //Set BIT0 of P1 as analog input
    ADC10AE0 = 0x01; //Select analog channel A0
    ADC10CTL1 = INCH_0 + ADC10DIV_3; //Set ADC clock to clock division 3
    ADC10CTL0 = ADC10SHT_3 + MSC + ADC10ON; //Turn ADC ON
    ADC10CTL0 |= ENC + ADC10SC; //Enable ADC
}

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a character as input and displays the character on a seven segment hex display
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////

void seg7Display(int input) {
    P2OUT = 0xFE;

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

    P2OUT = 0xFE;

    P1OUT &= ~0xF0;
    P1OUT |= 0x80;
    seg7Display(adcReading % 10);

    int adcReadingTemp = adcReading / 10;

    for(i = 0; i < 250; i++);

    if(adcReading > 9) {
        P2OUT = 0xFE;

        P1OUT &= ~0x80;
        P1OUT |= 0x40;
        seg7Display(adcReadingTemp % 10);
        adcReadingTemp /= 10;

        for(i = 0; i < 250; i++);
    }

    if(adcReading > 99) {
        P2OUT = 0xFE;

        P1OUT &= ~0x40;
        P1OUT |= 0x20;
        seg7Display(adcReadingTemp % 10);


        for(i = 0; i < 250; i++);
    }

    if(adcReading > 999) {
        P2OUT = 0xFE;

        P1OUT &= ~0x20;
        P1OUT |= 0x10;
        seg7Display(1);

        for(i = 0; i < 250; i++);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////
// Removes oscillations from ADC by only incrementing if ADC reading is above or below certain values
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////
int removeOscillations(int adcReading) {
    int next = ADC10MEM;

    if(next >= adcReading + 20) {
        adcReading += 1;
    } else if(next <= adcReading - 20) {
        adcReading -= 1;
    }

    if(next >= adcReading + 50) {
        adcReading = next;
    } else if(next <= adcReading - 50) {
        adcReading = next;
    }

    if(next >= 1019) {
        adcReading = 1023;
    } else if(next == 0) {
        adcReading = 0;
    }

    return adcReading;
}
