#include "buttons.h"
#include "common_makros.h"

#include <avr/io.h>

#define BUTOONPORT		PORTC
#define BUTOONDDRPORT	DDRC
#define BUTOONPINPORT	PINC


volatile
char button_pins[BUTTONTOTAL] = { 	5, // BUTTONDOOR
									4  // BUTTONNEWNOCK
								};

//=============================================================================
inline void gpioInit(){
	char i;
	for (i  = 0; i < BUTTONTOTAL; i++){
		DOWNBIT(BUTOONDDRPORT, 1 << button_pins[i]);	
		UPBIT(BUTOONPORT, 1 << button_pins[i]);	
	}
}
//=============================================================================
void buttonsInit(){
	gpioInit();
}
//=============================================================================
char buttonIsPressed(char button_name){
	return ((BUTOONPINPORT & (1 << button_pins[button_name])) == 0) ? TRUE : FALSE;
}
//=============================================================================
