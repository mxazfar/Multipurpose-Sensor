/* Gilbert Traczyk & Moosa Azfar
*
* This program takes the input from an ultrasonic sensor and displays it on a seven segment display by transmitting the decoded reading over UART to another
microcontroller.
* The reading range of 2-400 is divided into 5 ranges (2-50, 50-100, 100-200, 200-300, 300-400); each range corresponds to a different pitch on the buzzer. As distance
decreases, pitch increases.
*/

#include <msp430.h>

//GFEDCBAX
//76543210
//4321
//3210

int removeOscillations(int currentDistance);
void displayUC();
void sensorUC();
void seg7Display(int input);
void displayQuad7Seg(int adcReading);
void displayUCSetup();
void sensorUCSetup();
void setup();

//Definition of states for echo signal
typedef enum {
    RISING_EDGE,
    FALLING_EDGE
} echoTrigger_t;

typedef enum {
    DIST_1,
    DIST_2,
    DIST_3,
    DIST_4,
    DIST_5
} buzzerState_t;

int displaySelectedDistance = 1;
int presetDistances[5] = {10, 20, 50, 90, 200};

echoTrigger_t edge_state = RISING_EDGE;
buzzerState_t buzzer_state = DIST_1;

volatile unsigned int startStopTimestamps[2]; // Variable to keep track of rising and falling edge timestamps for echo signal
int displayVal = 0; //value displayed on quad-seven-segment
int newVal = 0; //digit to be updated in displayVal
int index = 0; //index of which digit to be transmitted
int changes = 0; //tracker of hysteresis for our hysteresis
int holdDistance = 0; //value to be passed to second chip
unsigned int distance; //Variable to hold distance calculation

//////////////////////////////////////////////////////////////////////////////////////////////
// Calls setup functions and enters do-forever loop
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void main(void)
{
     setup(); //General setup for parameters relevant to both microcontrollers

     if(P1IN & 0x08) {
         displayUCSetup(); //Setup for display micro
     } else {
         sensorUCSetup(); //Setup for sensor micro
     }

     while(1) {
         if(P1IN & 0x08) {
             displayUC();
         } else {
             sensorUC();
         }
     }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes parameters relevant to both microcontrollers
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
     WDTCTL = WDTPW | WDTHOLD;
     P1DIR &= ~0x08; // Set P1 bit 3 as input to check hardware flag
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

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes parameters relevant to display microcontroller
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void displayUCSetup() {
     P1DIR |= 0xF1; // Set P1 bits 4-7 as output
     P2SEL &= ~0xC0; //Set P2 bits 6 and 7 to digital output instead of XIN/XOUT
     P2DIR |= 0xFF; //Set entire P2 to output;
     P2OUT |= 0xFF; //Ensure all P2 bits are off
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Initializes parameters relevant to sensor microcontroller
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
void sensorUCSetup() {
     P1DIR = BIT6; // Set Trigger Bit to Output

     P2DIR &= ~BIT2; //Set P2.2 as input for echo
     P2IE |= BIT2; //Enable P2.2 interrupts
     P2IES &= ~BIT2; //Set P2.2 interrupt to rising edge
     P2IFG &= ~BIT2; //Clear P2.2 IFG

     P1DIR &= ~BIT0; //Set P1.0 as input for button
     P1IE |= BIT0; //Enable P1.0 interrupts
     P1IES |= BIT0; //Set P1.0 interrupt to falling edge
     P1REN |= BIT0;
     P1OUT |= BIT0;
     P1IFG &= ~BIT0; //Clear P1.0 IFG

     TA0CTL = TASSEL_2|MC_2; //Set timer A0 to count up
     TA0CCTL0 = CCIE; //Enable Timer A0 Interrupt
     TA0CCR0 = 0xFFFF; //Set timer to count up to ~65 ms

     P2DIR |= BIT1; // P2.1 set as output for PWM
     P2SEL |= BIT1; // P2.1 alternate function select

     TA1CCR0 = 0; // Set PWM period
     TA1CCTL1 |= OUTMOD_7; // Output mode to reset/set
     TA1CCR1 |= 750; // Set duty cycle
     TA1CTL |= TASSEL_2 + MC_1; // SMCLK, Up Mode (Counts to TA1CCR0)
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
// Removes oscillations from ultrasonic sensor by only incrementing if distance reading is above or below certain values
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////
int removeOscillations(int currentDistance) {
     int next = distance;
     int numChanges = 20;
     int bandWidth = (currentDistance/10)+5;

     if((next >= currentDistance + bandWidth) && (changes > numChanges)) {
         currentDistance += 1;
         changes = 0;
     } else if(next >= currentDistance + bandWidth){
         changes++;
     } else if((next <= currentDistance - bandWidth) && (changes < -numChanges)) {
         currentDistance -= 1;
         changes = 0;
     } else if(next <= currentDistance - bandWidth){
         changes--;
     }

     if((next >= currentDistance) && (currentDistance == 360)) {
         currentDistance = next;
     }

     return currentDistance;
}

//===================================================
// Takes the value received from the RX buffer and bit masks the updated digit into displayVal
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
// Reads distance from global variable and transmits it over UART digit by digit. The buzzer frequency is also set based on distance
//===================================================
    void sensorUC() {
     int data = 0;
     holdDistance = removeOscillations(holdDistance);

     if(displaySelectedDistance == 1) {
         volatile int i = 0;

         while(i < 0xFFF) {
             switch(index){
                  case 0:
                      data = (index<<4) |(presetDistances[buzzer_state])%10;//first 4 bits: index | last 4: first digit
                      UCA0TXBUF = data;
                      index = 1;
                      break;
                  case 1:
                      data = (index<<4) |(presetDistances[buzzer_state]/10)%10;//first 4 bits: index | last 4: second digit
                      UCA0TXBUF = data;
                      index = 2;
                      break;
                  case 2:
                      data = (index<<4) |(presetDistances[buzzer_state]/100)%10;//first 4 bits: index | last 4: third digit
                      UCA0TXBUF = data;
                      index = 3;
                      break;
                  case 3:
                      data = (index<<4) |(presetDistances[buzzer_state]/1000)%10;//first 4 bits: index | last 4: fourth digit
                      UCA0TXBUF = data;
                      index = 0;
                      break;
              }

             i++;
         }

         displaySelectedDistance = 0;
     }

     switch(index){
         case 0:
             data = (index<<4) |(holdDistance)%10;//first 4 bits: index | last 4: first digit
             UCA0TXBUF = data;
             index = 1;
             break;
         case 1:
             data = (index<<4) |(holdDistance/10)%10;//first 4 bits: index | last 4: second digit
             UCA0TXBUF = data;
             index = 2;
             break;
         case 2:
             data = (index<<4) |(holdDistance/100)%10;//first 4 bits: index | last 4: third digit
             UCA0TXBUF = data;
             index = 3;
             break;
         case 3:
             data = (index<<4) |(holdDistance/1000)%10;//first 4 bits: index | last 4: fourth digit
             UCA0TXBUF = data;
             index = 0;
             break;
     }

     if((holdDistance == presetDistances[DIST_1]) && (buzzer_state == DIST_1)) {
         TA1CCR0 = 1523;
     } else if((holdDistance == presetDistances[DIST_2]) && (buzzer_state == DIST_2)) {
         TA1CCR0 = 1716;
     } else if((holdDistance == presetDistances[DIST_3]) && (buzzer_state == DIST_3)) {
         TA1CCR0 = 1812;
     } else if((holdDistance == presetDistances[DIST_4]) && (buzzer_state == DIST_4)) {
         TA1CCR0 = 2035;
     } else if((holdDistance == presetDistances[DIST_5]) && (buzzer_state == DIST_5)) {
         TA1CCR0 = 2289;
     } else {
         TA1CCR0 = 0;
     }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Takes the value from the RX Buffer and stores it upon RX interrupt trigger; clears interrupt flag
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
     newVal = UCA0RXBUF;
     IFG2 &= ~UCA0RXIFG; // Clear RX flag
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Clears TX interrupt flag set when data is placed into TX buffer and interrupt is generated
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = USCIAB0TX_VECTOR
__interrupt void TransmitInterrupt(void) {
     IFG2 &= ~UCA0TXIFG; // Clear RX flag
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Takes Pin 1.6 high for exactly 10 us every 65 microseconds to generate trigger signal based on datasheet specifications
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void){
     P1OUT |= BIT6;
     __delay_cycles(6);
     P1OUT &= ~BIT6;
     TA0CCR0 = 0xFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Reads echo pin and stores start/stop time based on rising/falling edge. Computes distance on falling edge.
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void) {
     if(edge_state == RISING_EDGE) {
         startStopTimestamps[0] = TA0R; //Record start time
         P2IES |= BIT2; //Set P2.2 interrupt to falling edge
         edge_state = FALLING_EDGE; //Update state
     } else if(edge_state == FALLING_EDGE) {
         startStopTimestamps[1] = TA0R; //Record stop time
         P2IES &= ~BIT2; //Set P2.2 interrupt to rising edge

         distance = (startStopTimestamps[1] - startStopTimestamps[0]) / 58; //Calculates distance

         //Adjusts distance based on upper and lower specs of sensor
         if(distance > 400) {
             distance = 400;
         } else if(distance < 2) {
             distance = 2;
         }

         edge_state = RISING_EDGE; //Updates state
     }

     P2IFG &= ~BIT2; //Clear P2.2 IFG
}

//////////////////////////////////////////////////////////////////////////////////////////////
// ISR for button press; updates buzzer state flag cyclically.
// @mazfar@uncc.edu
//////////////////////////////////////////////////////////////////////////////////////////////
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    if(P1IFG & BIT0) {
        switch(buzzer_state) {
            case DIST_1:
                buzzer_state = DIST_2;
                break;
            case DIST_2:
                buzzer_state = DIST_3;
                break;
            case DIST_3:
                buzzer_state = DIST_4;
                break;
            case DIST_4:
                buzzer_state = DIST_5;
                break;
            case DIST_5:
                buzzer_state = DIST_1;
                break;
        }

        displaySelectedDistance = 1;

        P1IFG &= ~BIT0;
    }
}
