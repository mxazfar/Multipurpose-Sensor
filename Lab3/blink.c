/* Gilbert Traczyk & Moosa Azfar
*
* This program is to demonstrate the ability to program two separate microcontrollers with one codefile
*/

#include <msp430.h>

//GFEDCBAX
//76543210
//4321
//3210

int removeOscillations(int adcReading);
void displayUC();
void adcUC(int Sendvalue);
void seg7Display(int input);
void displayQuad7Seg(int adcReading);
void setup();

int displayVal = 0; //value displayed on quad-seven-segment
int newVal = 0; //digit to be updated in displayVal
int index = 0; //index of which digit to be transmitted
int changes = 0; //tracker of hysteresis for our hysteresis

//Initializes ports and ADC, polls for changes in potentiometer and updates display accordingly
void main(void)
{
    setup();
    volatile int i;
    volatile int j;

    //variables used to store ADC values
    int adcReading = ADC10MEM;
    int next = ADC10MEM;

    //variable used to reduce the number of adc updates
    int updateReduce = 0;

    while(1) {
        if(P1IN & 0x08) {
            displayUC();//displays received rx value
        } else {
        if(updateReduce>1){
            adcReading = removeOscillations(adcReading);//update ADC value
            ADC10CTL0 |= ENC + ADC10SC; //Enable ADC
            updateReduce=0;
        }
            updateReduce++;
            adcUC(adcReading);//transmit ADC value
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes ports and ADC
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= 0xF1; // Set P1 bits 4-7 as output

    P1DIR &= ~0x08; // Set P1 bit 3 as input
    P2SEL &= ~0xC0; //Set P2 bits 6 and 7 to digital output instead of XIN/XOUT
    P2DIR |= 0xFF; //Set entire P2 to output;
    P2OUT |= 0xFF; //Ensure all P2 bits are off
    P1SEL |= BIT0; //Set BIT0 of P1 as analog input

    ADC10AE0 = 0x01; //Select analog channel A0
    ADC10CTL1 = INCH_0 + ADC10DIV_3; //Set ADC clock to clock division 3
    ADC10CTL0 = ADC10SHT_3 + MSC + ADC10ON; //Turn ADC ON
    ADC10CTL0 |= ENC + ADC10SC; //Enable ADC

    DCOCTL = 0; // Select lowest DCOx and MODx settings<
    BCSCTL1 = CALBC1_1MHZ; // Set DCO
    DCOCTL = CALDCO_1MHZ;

    P1SEL |= BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0CTL0 |= 0x80; // Enable Parity
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    UC0IE |= UCA0RXIE + UCA0TXIE; // Enable USCI_A0 RX interrupt

    __enable_interrupt(); //Enable interrupts globally
}

///////////////////////////////////////////////////////////////////////////////////////////
// Takes in a character as input and displays the character on a seven segment hex display
// @mazfar@uncc.edu
///////////////////////////////////////////////////////////////////////////////////////////
void seg7Display(int input) {
    P2OUT = 0xFF;

    switch(input) {
        case 0:
            P2OUT &= ~(0x3F);
            break;
        case 1:
            P2OUT &= ~(0x06);
            break;
        case 2:
            P2OUT &= ~(0x5B);
            break;
        case 3:
            P2OUT &= ~(0x4F);
            break;
        case 4:
            P2OUT &= ~(0x66);
            break;
        case 5:
            P2OUT &= ~(0x6D);
            break;
        case 6:
            P2OUT &= ~(0x7D);
            break;
        case 7:
            P2OUT &= ~(0x07);
            break;
        case 8:
            P2OUT &= ~(0x7F);
            break;
        case 9:
            P2OUT &= ~(0x6F);
            break;
        case 10:
            P2OUT &= ~(0x77);
            break;
        case 11:
            P2OUT &= ~(0x7C);
            break;
        case 12:
            P2OUT &= ~(0x39);
            break;
        case 13:
            P2OUT &= ~(0x5E);
            break;
        case 14:
            P2OUT &= ~(0x79);
            break;
        case 15:
            P2OUT &= ~(0x71);
            break;
        default:
            P2OUT = 0xFF;
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

    P2OUT = 0xFF;
    P1OUT &= ~0xF0;
    P1OUT |= 0x80;

    seg7Display(adcReading % 10);

    int adcReadingTemp = adcReading / 10;

    for(i = 0; i < 250; i++);

    if(adcReading > 9) {
        P2OUT = 0xFF;
        P1OUT &= ~0x80;
        P1OUT |= 0x40;
        seg7Display(adcReadingTemp % 10);
        adcReadingTemp /= 10;
        for(i = 0; i < 250; i++);
    }

    if(adcReading > 99) {
        P2OUT = 0xFF;
        P1OUT &= ~0x40;
        P1OUT |= 0x20;
        seg7Display(adcReadingTemp % 10);
        for(i = 0; i < 250; i++);
    }

    if(adcReading > 999) {
        P2OUT = 0xFF;
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
    int numChanges = 10;
    int bandWidth = 20;

    if((next >= adcReading + bandWidth) && (changes > numChanges)) {
        adcReading += 1;
        changes = 0;
    } else if(next >= adcReading + bandWidth){
        changes++;
    } else if((next <= adcReading - bandWidth) && (changes < -numChanges)) {
        adcReading -= 1;
        changes = 0;
    } else if(next <= adcReading - bandWidth){
        changes--;
    }

    if(next > adcReading + 120) {
        adcReading = next;
    }
    if(next < adcReading - 120) {
        adcReading = next;
    }
    if(next >= 1021) {
        adcReading = 1023;
    } else if(next == 0 && changes < numChanges) {
        adcReading = next;
    }
    return adcReading;
}

//===================================================
// Takes the value received from the rx buffer and bit masks the updated digit into displayVal
//===================================================
void displayUC() {
    int updateData = newVal & 0b1111;
    int updateIndex = (newVal >> 4);
    int a=(displayVal/1000)%10, b=(displayVal/100)%10, c=(displayVal/10)%10, d=(displayVal)%10; //declares each digit of displayVal seperately to mask individually

    //switch case to update individual digit depending on received index
    switch(updateIndex){
        case 0:
            d=updateData;
            break;
        case 1:
            c=updateData;
            break;
        case 2:
            b=updateData;
            break;
        case 3:
            a=updateData;
            break;
    }

    displayVal = (a*1000)+(b*100)+(c*10)+d; //rebuilds updated displayVal
    displayQuad7Seg(displayVal);//displays
}

//=================================================
// Takes in integer input and places that input into the transmit buffer one digit at a time
// first 4 bits becomes the digit index and then last 4 become the digit transmitted
//===================================================
void adcUC(int sendValue) {
    int data = 0;

    switch(index){
        case 0:
            data = (index<<4) |(sendValue)%10;//first 4 bits: index | last 4: first digit
            UCA0TXBUF = data;
            index = 1;
            break;
        case 1:
            data = (index<<4) |(sendValue/10)%10;//first 4 bits: index | last 4: second digit
            UCA0TXBUF = data;
            index = 2;
            break;
        case 2:
            data = (index<<4) |(sendValue/100)%10;//first 4 bits: index | last 4: third digit
            UCA0TXBUF = data;
            index = 3;
            break;
        case 3:
            data = (index<<4) |(sendValue/1000)%10;//first 4 bits: index | last 4: fourth digit
            UCA0TXBUF = data;
            index = 0;
            break;
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
    newVal = UCA0RXBUF;
    IFG2 &= ~UCA0RXIFG; // Clear RX flag
}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void TransmitInterrupt(void) {
    IFG2 &= ~UCA0TXIFG; // Clear RX flag
}
