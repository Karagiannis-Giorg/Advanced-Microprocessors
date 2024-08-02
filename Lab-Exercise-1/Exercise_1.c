#include <avr/io.h>
#include <avr/interrupt.h>

#define T1 20 // περίοδος που περνάει το τρένο
#define T2 10  // περίοδος που είναι ανοιχτό το φανάρι των πεζών
#define T3 5 // περίοδος που που το κουμπί των πεζών ειναι ανενεργό

int x=0; //flag
int pedestrian_button = 0; // Mεταβλητή που δείχνει αν το κουμπί πεζών έχει απενεργοποιήθει (τώρα δείχνει On)
int train_passes = 0; // Mεταβλητή που δείχνει το τρένο περνάει εκείνη την στιγμή

void TRAIN_TCA0(void);
void SPLIT_TCA0(void);

void TRAIN_TCA0(){// Χρονιστής που μετράει την περίοδο T1 για τα τρένα
	TCA0.SINGLE.CNT = 0; //
	TCA0.SINGLE.CTRLB = 0;
	TCA0.SINGLE.CMP0 = T1;
	TCA0.SINGLE.CTRLA = 0x7<<1;
	TCA0.SINGLE.CTRLA |=1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
}


void SPLIT_TCA0(){// Χρονιστής που μετράει την περίοδο Τ2 & Τ3
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // ενεργοποίηση split mode για τον TCA0
	TCA0.SPLIT.LCNT = 0;
	TCA0.SPLIT.HCNT = 0;
	TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_LCMP0EN_bm;
	TCA0.SPLIT.LCMP0 = T2;
	TCA0.SPLIT.HCMP0 = T3;
	TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc | TCA_SPLIT_ENABLE_bm;
	TCA0.SPLIT.INTCTRL = 0b00000001 | 0b00000010;
}


int main() {
	PORTD.DIR |= PIN0_bm|PIN1_bm|PIN2_bm; // Αρχικοποίηση LED που χρησιμοποιούμε
	PORTD.OUTCLR = PIN2_bm; // led-αυτοκινήτου on
	PORTD.OUT |= PIN0_bm|PIN1_bm; // led-πεζών & led-τρένου off
	TRAIN_TCA0();  //Αρχικοποίηση χρονιστή για τα τρένα
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	sei(); // ενεργοποίηση διακοπών
	while (x==0) {
		;
	}
	cli();
}


ISR(TCA0_CMP0_vect){  //interrupt οταν περνάει το τρένο
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;
	PORTD.OUTCLR = PIN0_bm|PIN1_bm;
	PORTD.OUT |= PIN2_bm;
	SPLIT_TCA0();   //Αρχικοποίηση χρονιστή για το T2 που χρειαζόμαστε για τους πεζούς όταν περνάνε τα τρένα
	train_passes = 1;
}


ISR(TCA0_LUNF_vect){ //interrupt οταν οταν το φανάρι των πεζών ειναι αναμένο
	TCA0.SPLIT.INTFLAGS=0b00000001;
	if(train_passes==1){ // περίπτωση που το φανάρι των πεζών είναι αναμένο όταν περνάει το τρένο
		PORTD.OUTCLR = PIN2_bm;
		PORTD.OUT |= PIN0_bm|PIN1_bm;
		TRAIN_TCA0();
		train_passes = 0;
	}else{ // περίπτωση που το φανάρι των πεζών είναι αναμένο όταν δεν περνάει το τρένο
		PORTD.OUTCLR = PIN2_bm;
		PORTD.OUT |= PIN0_bm;
		pedestrian_button = 1;
	}
}


ISR(TCA0_HUNF_vect){ //interrupt για το διάστημα που το κουμπί των πεζών ειναι ανενεργό
	TCA0.SPLIT.INTFLAGS=0b00000010;
	pedestrian_button = 0;
	TRAIN_TCA0();
}


ISR(PORTF_PORT_vect){//interrupt για το όταν πατηθεί το κουμπί των πεζών
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	if(train_passes ==1 || pedestrian_button == 1)// όταν το κουμπί έχει ήδη πατηθεί ή περνάει το τρένο δεν έχει καμία λειτουργεία
	;
	else{// όταν το κουμπί πατηθεί δλδ γινει το interupt
		SPLIT_TCA0();
		PORTD.OUTCLR = PIN0_bm;
		PORTD.OUT |= PIN2_bm;
	}
}
