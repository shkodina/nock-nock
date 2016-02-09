#include "softtimer.h"
#include "common_makros.h"

#include <avr/io.h>
#include <avr/interrupt.h>
//=============================================================================
enum {	TIMER_STATE_IN_RUN = 1,
		TIMER_STATE_ELAPSED = 2,
		TIMER_STATE_NEED_RESTART = 4
	};
//=============================================================================
typedef struct {
	unsigned int counter;
	unsigned int threshold;
	char state;
} SoftTimer;
//=============================================================================
volatile
SoftTimer g_timer[TIMER_TOTAL_COUNT];
//=============================================================================
void timerSet(char timer_name, unsigned int start_value, unsigned int stop_value){
	g_timer[timer_name].counter = start_value;
	g_timer[timer_name].threshold = stop_value;
	g_timer[timer_name].state = TIMER_STATE_IN_RUN;
}
//=============================================================================
char timerIsElapsed(char timer_name){
	if ((g_timer[timer_name].state & TIMER_STATE_ELAPSED) > 0){
		g_timer[timer_name].state &= ~TIMER_STATE_ELAPSED;
		return TRUE;
	}
	return FALSE;
}
//=============================================================================
unsigned int timerGetCurrent(char timer_name){
	return g_timer[timer_name].counter;
}
//=============================================================================
void timerInit(){
	char i;
	for (i = 0; i < TIMER_TOTAL_COUNT; i++){
		g_timer[i].counter = 0;
		g_timer[i].threshold = 0;
		g_timer[i].state = 0;
	}

	initHardWareTimer();
}
//=============================================================================
inline inIRQTimer(){
	char i;
	for (i = 0; i < TIMER_TOTAL_COUNT; i++){
		if ( (g_timer[i].state & TIMER_STATE_IN_RUN) > 0){
			if (g_timer[i].counter++ >= g_timer[i].threshold){
				g_timer[i].state &=	~TIMER_STATE_IN_RUN;
				g_timer[i].state |= TIMER_STATE_ELAPSED;
			}
		}
	}
}
//=============================================================================
//=============================================================================
//      HARDWARE      HARDWARE      HARDWARE      HARDWARE      HARDWARE
//=============================================================================
//=============================================================================
void initHardWareTimer()
{
    TCNT1 = 65536-1;        
    TCCR1B = (1<<CS12 | 1<<CS10);
    TIMSK |= (1<<TOIE1);
 	sei();
}
//=============================================================================
ISR (TIMER1_OVF_vect)
{
    inIRQTimer();
	// run timer
	TCNT1 = 65536 - 7; //  31220;
    TCCR1B = (1<<CS12 | 1<<CS10);
    TIMSK |= (1<<TOIE1);


}
//=============================================================================
