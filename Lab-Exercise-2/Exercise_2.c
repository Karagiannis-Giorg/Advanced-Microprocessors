#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int corner_counter = 0; // μέτρηση γωνιών που έχει διαπεράσει η σκούπα
int mode = 0; // ειναι απερνεγοποιημένο το free runing mode
int reverse_mode = 0; 

void init_TCA0(int T){
	TCA0.SINGLE.CNT = 0;
	TCA0.SINGLE.CTRLB = 0;
	TCA0.SINGLE.CMP0 = T;
	TCA0.SINGLE.CTRLA = 0x7<<1;
	TCA0.SINGLE.CTRLA |=1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
}

int main(){
	PORTD.DIR |= PIN0_bm| PIN1_bm | PIN2_bm;
	PORTD.OUT |= PIN0_bm | PIN2_bm;
	PORTD.OUTCLR = PIN1_bm;
	
	//initialize the ADC for single mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA = ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	//Window Comparator Mode
	ADC0.WINLT |= 3; //Set threshold
	ADC0.WINHT |= 5; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	//Interrupt 
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	init_TCA0(1);
    if (reverse_mode == 0){
		while(corner_counter <8 && reverse_mode == 0){
			sei();
		}cli();
	}
	if(reverse_mode == 1){
		while(corner_counter >0 && reverse_mode == 1){
			sei();
		}
	}
	return 0;
}

ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	if(reverse_mode == 0){
		if(mode == 1){
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN2_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN2_bm;
			corner_counter++;
		}else{
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN0_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN0_bm;
			corner_counter++;
		}
	}else{
		if(mode == 0){
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN2_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN2_bm;
			corner_counter--;
		}else{
			PORTD.OUT |= PIN1_bm;
			PORTD.OUTCLR = PIN0_bm;
			_delay_ms(1);
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUT |= PIN0_bm;
			corner_counter--;
		}
	}
}

ISR(TCA0_CMP0_vect){// interrupt για εναλλαγή μετακύ του πλαινού και μπροστινού αισθητήρα 
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;
	if(mode == 0){ // Ενάρξη διαβάσματος απο τον μπροστινό αισθητήρα 
		ADC0.CTRLA |= ADC_FREERUN_bm;
		ADC0.CTRLE = 0b00000001; //Interrupt when RESULT < WINLT
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		
		init_TCA0(2);
		mode = 1;
	}else{// Ενάρξη διαβάσματος απο τον πλαινό αισθητήρα
		ADC0.CTRLA = ADC_ENABLE_bm; //Enable ADC
		ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
		ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
		
		init_TCA0(1);
		mode = 0;
		
	}
}

ISR(PORTF_PORT_vect){
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
