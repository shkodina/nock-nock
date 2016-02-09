#ifndef MY_FUCKING_DOOR
#define MY_FUCKING_DOOR

#include <avr/io.h>

#define DOORPORT PORTC
#define DOORDDRPORT DDRC
#define DOORPIN 1

inline void doorOpen(){
	DOORPORT &= ~(1 << DOORPIN);
}

inline void doorClose(){
	DOORPORT |= (1 << DOORPIN);
}

inline void doorInit(){
	DOORDDRPORT |= (1 << DOORPIN);
	doorClose();
}

#endif
