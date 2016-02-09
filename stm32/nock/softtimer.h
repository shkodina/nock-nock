#ifndef MY_SOFT_TIMER_H
#define MY_SOFT_TIMER_H

enum {	TIMER_TOTAL_COUNT = 4,
		TIMER_NOCK = 0,
		TIMER_NOCK_NOISE = 1,
		TIMER_NOCK_TIMEOUT = 2,
		TIMER_RADIO_DELAY = 3
	};

enum {	TIMER_VALUE_NOCK_TIMEOUT = 1000,
		TIMER_VALUE_NOISE = 5,
		TIMER_VALUE_RADIO_DELAY = 5,
		TIMER_VALUE_MAX = 65535,
		TIMER_VALUE_INIT_ZERRO = 0
	};

//-------------------------------------------------------------------

void timerInit();
void timerSet(char timer_name, unsigned int start_value, unsigned int stop_value);
char timerIsElapsed(char timer_name);
unsigned int timerGetCurrent(char timer_name);


#endif
