#include <msp430.h> 

/*
 * Audrey Lewis
 * Caroline Ekchian
 *
 * This code will receive input from an array of photoresistors through the ADC. It will process these and then determine which
 * position has the greatest amount of light and then, using a stepper motor controlled by the timer, it will turn the motor to
 * face that direction.
 *
 */

#include "msp430g2553.h"

#define TA0_BIT 0x02
// define the location for the button (this is the built in button)
// specific bit for the button
#define BUTTON_BIT1 0x08
#define BUTTON_BIT2 0x20
#define BUTTON_BIT3 0x10
#define BUTTON_BIT4 0x04
//----------------------------------
//1000 makes it 500 Hz
//1500 makes it 250 Hz

#define initialHalfPeriod 1500;

volatile unsigned int numTicks = 0;
volatile unsigned int pressed = 0;
void init_timer(void); // routine to setup the timer
void init_button(void); // routine to setup the button

// ++++++++++++++++++++++++++
void main(){
	  WDTCTL =(WDTPW + // (bits 15-8) password
	                   // bit 7=0 => watchdog timer on
	                   // bit 6=0 => NMI on rising edge (not used here)
	                   // bit 5=0 => RST/NMI pin does a reset (not used here)
	           WDTTMSEL + // (bit 4) select interval timer mode
	           WDTCNTCL +  // (bit 3) clear watchdog timer counter
	  		          0 // bit 2=0 => SMCLK is the source
	  		          +1 // bits 1-0 = 01 => source/8K
	  		   );
	  IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

	  BCSCTL1 = CALBC1_8MHZ;    // 8Mhz calibration for clock

	  DCOCTL  = CALDCO_8MHZ;
	init_timer();  // initialize timer
	init_button(); // initialize the button
	_bis_SR_register(GIE+LPM0_bits);// enable general interrupts and power down CPU
}

void init_timer(){              // initialization and start of timer
	TACTL |= TACLR;              // reset clock
	TACTL = TASSEL_2+ID_3+MC_1+TAIE;  // clock source = SMCLK
	                            // clock divider=8
	                            // UP mode
	                            // timer A interrupt off
	TACCTL1 = CM_2+CAP+CCIE; //capture mode, falling edge on CCI1A, enable interrupt 1 <- TAIV of 2 when interrupted
	TA0CCTL0=0; // compare mode, output 0, no interrupt enabled
	TA0CCR0= initialHalfPeriod;
	P1SEL|=TA0_BIT; // connect timer output to pin
	P1DIR|=TA0_BIT;

}

void init_button(){
// All GPIO's are already inputs if we are coming in after a reset
	P1OUT |= BUTTON_BIT1; // pullup
	P1REN |= BUTTON_BIT1; // enable resistor
	P1IES |= BUTTON_BIT1; // set for 1->0 transition
	P1IFG &= ~BUTTON_BIT1;// clear interrupt flag
	P1IE  |= BUTTON_BIT1; // enable interrupt
}



void interrupt button_handler(){
// check that this is the correct interrupt
// (if not, it is an error, but there is no error handler)

	if (~P1IN & (BUTTON_BIT1)) {
		TACCTL0 |= OUTMOD_4;
		pressed = 1;
	} else {
		TACCTL0 &= ~OUTMOD_4;
	}
}

/*
void interrupt TA_handler() {
	//called for channel 1 interrupt, or overflow interrupt
	switch (TAIV) {
	case 2: {
//		numTicks = 0;
		/*
		if (TACCTL1 & CAP) {
			P1OUT = 0;
			//we will be in input capture mode
			//store the time (in long variable)
			union a {
				unsigned long L;
				unsigned int words[2];
			} capture_time;
			capture_time.words[0] = TACCR1;
			capture_time.words[1] = overflows;
			last_button_press_time = capture_time.L;

			//Calculate the total reaction time
			total_time = last_button_press_time - start_time;
			//Reset delay time
			rand_delay = rand()%1500;
			delay = 2000 + rand_delay;
			//Turn off LED

			last_button_press_time = 0;
			start_time = 0;
			overflows = 0;
			TACCTL1 &= ~(CAP+COV); //clear capture/compare overflow flags
		} else {
			TACCTL1 |= CAP; //go back to capture mode
		}
	}
	break;
	case 10: { //this is for an interrupt overflow
		if (numTicks > 100) {
		TACCTL0 &= ~OUTMOD_4;
		} else {
			if (pressed == 1) {
				numTicks++;
			}
		}
	}
	}
	}

}
ISR_VECTOR(TA_handler,".int08")
*/
/*
interrupt void WDT_interval_handler(){
	if (pressed == 1) {
		if (numTicks > 200){
			TACCTL0 &= ~OUTMOD_4;
		} else {
			numTicks++;
		}
	}
}
*/
//ISR_VECTOR(WDT_interval_handler, ".int10")
ISR_VECTOR(button_handler,".int02") // declare interrupt vector
//ISR_VECTOR(TA_handler,".int08")
