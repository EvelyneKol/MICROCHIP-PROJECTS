#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int corner_counter = 0; // μέτρηση γωνιών που έχει διαπεράσει η συσκευή
int mode = 0; // flag για Single Mode(0) και Free Running Mode(1)
int reverse_mode = 0; //flag για ανάποδη πορεία

void init_TCA0(int T){// Χρονιστής που μετράει την περίοδο T1=1ms και T2=2ms για τον ADC
	TCA0.SINGLE.CNT = 0;
	TCA0.SINGLE.CTRLB = 0;
	TCA0.SINGLE.CMP0 = T;
	TCA0.SINGLE.CTRLA = 0x7<<1;
	TCA0.SINGLE.CTRLA |=1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
}

int main(){
	PORTD.DIR |= PIN0_bm| PIN1_bm | PIN2_bm;//χρήση PIN0, PIN1, PIN2 για δεξιά, ευθεία και αριστερά στροφή
	PORTD.OUT |= PIN0_bm | PIN2_bm;
	PORTD.OUTCLR = PIN1_bm;//αρχικά άνοιγμα LED για ευθείας πορείας
	
	//Aρχικοποίηση του ADC σε single mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA = ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	//Window Comparator Mode
	ADC0.WINLT |= 3; //Set threshold for free Running Mode
	ADC0.WINHT |= 5; //Set threshold for Single Mode
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	//Interrupt 
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	init_TCA0(1);
    if (reverse_mode == 0){ // κανονική πορεία εφόσον το κουμπί για ανάποδη δεν έχει πατηθεί
		while(corner_counter <8 && reverse_mode == 0){//η συσκευή θα τερματίσει όταν ο αριθμός γωνιών γίνει 8
			sei();
		}cli();
	}
	if(reverse_mode == 1){ //το κουμπί για ανάποδη πορεία έχει πατηθεί 
		while(corner_counter >0 && reverse_mode == 1){ //η συσκευή θα τερματίσει όταν μηδενιστεί ο αριθμός γωνιών
			sei();
		}
	}
	return 0;
}

ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	if(reverse_mode == 0){ //κανονική πορεία 
		if(mode == 1){//διάβασε μη επιτρεπτή τιμή από τον μπροστινό αισθητήρα
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN2_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN2_bm;
			corner_counter++;//η συσκευή αυξάνει σε κάθε στροφή τον αριθμό γωνιών μέχρι να φτάσει τις 8 και να τερματίσει
		}else{//διάβασε μη επιτρεπτή τιμή από τον πλαϊνό αισθητήρα
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN0_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN0_bm;
			corner_counter++;//η συσκευή αυξάνει σε κάθε στροφή τον αριθμό γωνιών μέχρι να φτασει τις 8 και να τερματίσει
		}
	}else{//ανάποδη πορεία 
		if(mode == 0){
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN2_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN2_bm;
			corner_counter--;//μείωση σε κάθε στροφή του αριθμού γωνιών, ξεκινώντας από τον αριθμό γωνιών που είχε μετρηθεί όταν πατήθηκε το κουμπί
		}else{
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN0_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN0_bm;
			corner_counter--;//μείωση σε κάθε στροφή του αριθμού γωνιών, ξεκινώντας από τον αριθμό γωνιών που είχε μετρηθεί όταν πατήθηκε το κουμπί
		}
	}
}

ISR(TCA0_CMP0_vect){// interrupt για εναλλαγή μεταξύ του πλαϊνού και μπροστινού αισθητήρα 
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;
	if(mode == 0){ // Ενάρξη διαβάσματος από τον μπροστινό αισθητήρα 
		ADC0.CTRLA |= ADC_FREERUN_bm;
		ADC0.CTRLE = 0b00000001; //Interrupt when RESULT < WINLT
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		
		init_TCA0(2);
		mode = 1;
	}else{// Έναρξη διαβάσματος από τον πλαϊνό αισθητήρα
		ADC0.CTRLA = ADC_ENABLE_bm; //Enable ADC
		ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		
		init_TCA0(1);
		mode = 0;
		
	}
}

ISR(PORTF_PORT_vect){// interrupt με το πάτημα του κουμπιού
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y; 
	PORTD.OUTCLR = PIN0_bm|PIN1_bm|PIN2_bm;
	init_TCA0(5);
	PORTD.OUTCLR = PIN0_bm|PIN2_bm;
	ADC0.CTRLA = ADC_ENABLE_bm; //Enable ADC
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		
	init_TCA0(1);
	mode = 0;
	reverse_mode =1;
}
