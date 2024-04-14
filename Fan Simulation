#include <avr/io.h>
#include <avr/interrupt.h>

#define PER_VALUE_LOW        (0x14) // 20 Hz για λεπίδα
#define PER_VALUE_HIGH        (0x28) //40 Hz για βάση
#define DUTY_CYCLE_VALUE_L    (0x0A) //10(50% duty cycle για λεπίδα)
#define DUTY_CYCLE_VALUE_H    (0x18)//24 (60% duty cycle για βάση)
#define SECOND_PER_VALUE_LOW        (0x0A) // 10 Hz λεπίδα για 2ο ερώτημα
#define SECOND_DUTY_CYCLE_VALUE_L    (0x05) //5 (50% duty cycle για λεπίδα για 2ο ερώτημα )

int blade_cnt= 0; //flag για μέτρηση ακμών της λεπίδας
int base_cnt= 0;  //flag για μέτρηση ακμών της βάσης
int counter_switch = 0;  //flag για μέτρηση των φορών που έχει πατηθεί ο διακόπτης
int x=0;


int main(){
	
	PORTD.DIR = PIN0_bm| PIN1_bm; //Ενεργοποίηση blade pin & base pin
	PORTD.OUT |= PIN0_bm| PIN1_bm;//blade pin & base pin are off
	
	//enable inerrupts για τον διακόπτη
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm |PORT_ISC_BOTHEDGES_gc;
	sei();
	while (x==0)
	{
		;
	}
}

ISR(TCA0_LUNF_vect) {//interrupt όταν ο χρονιστής πάρει την τιμή LCMP0
	// Αν έχει προκληθεί hcount interrupt
	if ((TCA0.SPLIT.INTFLAGS & 0b00000010)==2){
		base_cnt++; // Αύξηση του flag κατά ένα
		if(base_cnt%2==0){//έλεγχος αν το base_cnt έχει άρτια τιμή
			PORTD.OUT |= PIN1_bm; //αν ναί τότε: base pin is off
		}
		else{
			PORTD.OUTCLR = PIN1_bm; //αν όχι τότε: base pin is on
		}
	}
	
	// Αν έχει προκληθεί lcount interrupt
	if ((TCA0.SPLIT.INTFLAGS & 0b00000001)==1){
		blade_cnt++; // Αύξηση του flag κατά ένα
		if(blade_cnt%2==0){ //έλεγχος αν το blade_cnt έχει άρτια τιμή
			PORTD.OUT |= PIN0_bm; // αν ναί τότε: blade pin is off
		}
		else{
			PORTD.OUTCLR = PIN0_bm; //αν όχι τότε: blade pin is on
		}
	}
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS = intflags;// καθαρισμός των INTFLAGS
}

ISR(PORTF_PORT_vect){//interrupt για όταν πατηθεί ο διακόπτης
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	counter_switch++; // αύξηση του flag κατά 1 κάθε φορά που πατιέται ο διακόπτης
	
	if(counter_switch==1){// αν ο διακόπτης έχει πατηθεί μία φορά
		
		TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // ενεργοποίηση split mode για τον TCA0
		TCA0.SPLIT.CTRLB  = TCA_SPLIT_LCMP0EN_bm|TCA_SPLIT_HCMP0EN_bm;
		TCA0.SPLIT.CTRLA  = TCA_SPLIT_CLKSEL_DIV1024_gc;
		
		/* Ορισμός συσχνοτήτων και κύκλων λειρτουργίας του PWM */
		TCA0.SPLIT.LPER = PER_VALUE_LOW; //Εφαρμογή συσχότητας για τα LOW bit του TCAO
		TCA0.SPLIT.LCMP0 = DUTY_CYCLE_VALUE_L; // Db=50% για τα LOW bit του TCAO
		
		TCA0.SPLIT.HPER = PER_VALUE_HIGH; //Εφαρμογή συσχότητας για τα HIGH bit του TCAO
		TCA0.SPLIT.HCMP0 = DUTY_CYCLE_VALUE_H; //Db=60% για τα HIGH bit του TCAO
		
		TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm; // LUNF INTERRUPT ENABLE
		TCA0.SPLIT.CTRLA  |= TCA_SPLIT_ENABLE_bm; //Enable τον TCA0
		
		}else if(counter_switch==2){// αν ο διακόπτης έχει πατηθεί δύο φορές
		
		TCA0.SPLIT.CTRLA &= ~(TCA_SPLIT_ENABLE_bm); // stop χρονιστή
		TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc; // Reset χρονιστή
		
		//καθαρισμός των flags
		blade_cnt=0;
		base_cnt=0;
		
		TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // ενεργοποίηση split mode για τον TCA0
		TCA0.SPLIT.CTRLB  = TCA_SPLIT_LCMP0EN_bm|TCA_SPLIT_HCMP0EN_bm;
		TCA0.SPLIT.CTRLA  = TCA_SPLIT_CLKSEL_DIV1024_gc;
		
		/* Ορισμός συσχνοτήτων και κύκλων λειρτουργίας του PWM */
		TCA0.SPLIT.LPER = SECOND_PER_VALUE_LOW; //Εφαρμογή υποδιπλάσιας συσχότητας για τα LOW bit του TCAO
		TCA0.SPLIT.LCMP0 = SECOND_DUTY_CYCLE_VALUE_L; // Db=50% για τα LOW bit του TCAO
		
		TCA0.SPLIT.HPER = PER_VALUE_HIGH; //Εφαρμογή ίδιας συσχότητας για τα HIGH bit του TCAO
		TCA0.SPLIT.HCMP0 = DUTY_CYCLE_VALUE_H; //Db=60% για τα HIGH bit του TCAO
		
		TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm; // LUNF INTERRUPT ENABLE
		TCA0.SPLIT.CTRLA  |= TCA_SPLIT_ENABLE_bm; //Enable τον TCA0

		}else if(counter_switch==3){// αν ο διακόπτης έχει πατηθεί τρείς φορές
		TCA0.SPLIT.CTRLA &= ~(TCA_SPLIT_ENABLE_bm); // stop χρονιστή
		TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc; // Reset χρονιστή
		
		PORTD.OUT |= PIN0_bm|PIN1_bm; //blade pin & base pin are off
		
		//καθαρισμός των flags
		blade_cnt=0;
		base_cnt=0;
		counter_switch=0;
	}
}
