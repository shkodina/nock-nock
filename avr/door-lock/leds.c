#include "leds.h"
#include "common_makros.h"

#include <avr/io.h>

#define LEDPORT		PORTD
#define LEDDDRPORT	DDRD

volatile
char led_pins[LEDTOTAL] = {	3, 	// LEDGREEN1
            		  	  	4, 	// LEDRED1
            		  	  	7, 	// LEDGREEN2
            		  	  	6 	// LEDRED2
              	  	  	  };



//=============================================================================
void ledInit(){
	char i;
	for (i  = 0; i < LEDTOTAL; i++){
		UPBIT(LEDDDRPORT, led_pins[i]);	
		DOWNBIT(LEDPORT, led_pins[i]);	
	}
}
//=============================================================================
void ledOn(char led_name){
	UPBIT(LEDPORT, led_pins[led_name]);
}
//=============================================================================
void ledOff(char led_name){
	DOWNBIT(LEDPORT, led_pins[led_name]);
}//=============================================================================
void ledTaggle(char led_name){
	INVBIT(LEDPORT, led_pins[led_name]);
}//=============================================================================
