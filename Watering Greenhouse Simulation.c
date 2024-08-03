#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define PER_VALUE        (0x14) // 20 Hz για το LED2
#define DUTY_CYCLE_VALUE   (0x0A) //10 -> (50% DC για το LED2)

int low_humity_thres =5; // κατώτατο threshold για την υγρασία
int high_humdity_thres=10; // υψηλότερο threshold για την υγρασία
int watering_plants = 0; // flag ένδειξης ότι τα φυτά πρέπει να ποτιστούν
int ventilation = 0; // flag ένδειξης ότι πρέπει να γίνει αερισμός του θερμοκηπιού
int cnt_rising_edges =0; // μετρητής ανερχόμενων ακμών PWM
int x=0; // flag ώστε να μην τελειώνει το πρόγραμμα μας
int adcVal; // μεταβλητή στην οποία αποθηκεύεται η τιμη του RES που προκάλεσαι το interupt

int main(){
	// ορισμός των led που θα χρησιμοποιήσουμε
	PORTD.DIR |= PIN0_bm| PIN1_bm | PIN2_bm;
	PORTD.OUT |= PIN0_bm |PIN1_bm| PIN2_bm;
	
	// ενεργοποίηση των switch 5 & 6
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	//initialize the ADC for single mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	
	//Window Comparator Mode
	ADC0.WINLT |= low_humity_thres; //Κατώτατο threshold
	ADC0.WINHT |= high_humdity_thres; //Υψηλότερο  threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= 0b00000100; //Interrupt when RESULT < WINLT or RESULT > WINHT
	sei(); //enable interrupts
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	
	while(x==0){
		;
	}
}

ISR(ADC0_WCOMP_vect){ // interrupt όταν διαβαστεί μη επιτρεπτή τιμή
	adcVal = ADC0.RES; // διάβασμα τιμής RES
	
	if (adcVal < low_humity_thres) // έλεγχος πρόκλησης interrupt από χαμηλά επίπεδα υγρασίας
	{
		PORTD.OUTCLR = PIN0_bm; // LEDO -> on
		watering_plants =1; // ενεργοποίηση flag ένδειξης ποτίσματος
	}
	
	if (adcVal > high_humdity_thres) // έλεγχος πρόκλησης interrupt από υψηλά επίπεδα υγρασίας
	{
		PORTD.OUTCLR = PIN1_bm; // LED1 -> on
		ventilation=1; // ενεργοποιηση flag ένδειξης αερισμού
	}
	
	// καθαρισμός των interrupt Flags
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
}


ISR(PORTF_PORT_vect){ // interrupt όταν πατηθεί ένα από switch (5 ή 6)
	if ((PORTF.INTFLAGS & 0b00100000)==0x20) // interrupt όταν πατηθεί το switch 5
	{
		if(watering_plants == 1 && ventilation == 0){ // ενεργοποίηση του switch 5 όταν πρέπει να γίνει πότησμα
			TCA0.SINGLE.CNT = 0;
			TCA0.SINGLE.CTRLB = 0;
			TCA0.SINGLE.CMP0 = (low_humity_thres-adcVal); // χρόνος λειτοργίας του timer
			TCA0.SINGLE.CTRLA = 0x7<<1;
			TCA0.SINGLE.CTRLA |=1;
			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
			}else if(ventilation == 1 && watering_plants == 0){// λανθασμένη ενεργοποίηση του switch 5 όταν πρέπει να γίνει αερισμός
			PORTD.OUTCLR = PIN0_bm | PIN2_bm; // ενεργοποίηση όλων των Leds
			_delay_ms(2); // βηματική εναλλαγή
			PORTD.OUT |= PIN0_bm | PIN2_bm; // απενεργοποίηση των leds που άναψαν
		}
	}
	
	if ((PORTF.INTFLAGS & 0b01000000)==0x40) // interrupt όταν πατηθεί το switch 6
	{
		if(ventilation == 1 && watering_plants == 0){ // ενεργοποίηση του switch 6 όταν πρέπει να γίνει αερισμός
			TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
			TCA0.SINGLE.PER = PER_VALUE; //select the resolution
			TCA0.SINGLE.CMP0 = DUTY_CYCLE_VALUE; //select the duty cycle
			//select Single_Slope_PWM
			TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
			//enable interrupt Overflow
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
			//enable interrupt COMP0
			TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
			TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
			}else if(watering_plants == 1 && ventilation == 0){ // λανθασμένη ενεργοποίηση του switch 6 όταν πρέπει να γίνει πότησμα
			
			PORTD.OUTCLR = PIN1_bm| PIN2_bm; // ενεργοποίηση όλων των Leds
			_delay_ms(2);// βηματική εναλλαγή
			PORTD.OUT |= PIN1_bm| PIN2_bm;// απενεργοποίηση των leds που άναψαν
		}
	}
	// καθαρισμός των interrupt flags
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
}

ISR(TCA0_CMP0_vect){ // interrupt όταν μετρηθεί η τιμή του TCA0
	if (watering_plants == 1) // αν είμαστε σε κατάσταση ποτίσματος
	{
		PORTD.OUT |= PIN0_bm;// LED0 -> off
		watering_plants =0;// καθαρισμός του flag για το πότισμα
		
		// καθαρισμός των interrupt flags
		int intflags = TCA0.SINGLE.INTFLAGS;
		TCA0.SINGLE.INTFLAGS=intflags;
		
		// stop & reset του TCA0 για επαναχρησιμοποίηση
		TCA0.SPLIT.CTRLA &= ~(TCA_SPLIT_ENABLE_bm);
		TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc;
	}
	
	if (ventilation == 1) // αν είμαστε σε κατάσταση αερισμού
	{
		cnt_rising_edges++; // Αύξηση του μετρητή των ανοδικών ακμών του PWM κατά 1
		PORTD.OUT |= PIN2_bm; // LED2 -> oFF
		
		// καθαρισμός των interrupt flags
		int intflags = TCA0.SINGLE.INTFLAGS;
		TCA0.SINGLE.INTFLAGS=intflags;
	}
}

ISR(TCA0_OVF_vect){
	if (cnt_rising_edges<4){ // συνέχιση του PWM μέχρι να μετρήσουμε 4 ανωδικές ακμές
		cnt_rising_edges++;// Αύξηση του μετρητή των ανοδικών ακμών του PWM κατά 1
		PORTD.OUTCLR = PIN2_bm;// LED2 -> on
		
		// καθαρισμός των interrupt flags
		int intflags = TCA0.SINGLE.INTFLAGS;
		TCA0.SINGLE.INTFLAGS=intflags;
		}else{// όταν έχουν μετρηθεί 4 ανωδικές ακμές
		PORTD.OUT |= PIN1_bm|PIN2_bm; // LED1,LED2 -> oFF
		ventilation=0; // καθαρισμός του flag για τον αερισμό
		
		// καθαρισμός των interrupt flags
		int intflags = TCA0.SINGLE.INTFLAGS;
		TCA0.SINGLE.INTFLAGS=intflags;
		// stop & reset του TCA0 για επαναχρησιμοποίηση
		TCA0.SPLIT.CTRLA &= ~(TCA_SPLIT_ENABLE_bm);
		TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc;
	}
}
