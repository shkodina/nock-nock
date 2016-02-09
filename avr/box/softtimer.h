#ifndef MY_SOFT_TIMER_H
#define MY_SOFT_TIMER_H

enum {	TIMER_NOCK = 0,
		TIMER_NOCK_NOISE = 1,
		TIMER_NOCK_TIMEOUT = 2,
		TIMER_RADIO_WAIT = 3,
		TIMER_DOOR_OPEN = 4,
		TIMER_ERROR = 5,
		TIMER_TOTAL_COUNT = 6
	};

enum {	TIMER_VALUE_NOCK_TIMEOUT = 1500,
		TIMER_VALUE_NOISE = 10,
		TIMER_VALUE_RADIO_WAIT = 5,
		TIMER_VALUE_MAX = 65535,
		TIMER_VALUE_INIT_ZERRO = 0,
		TIMER_VALUE_DOOR_OPEN = 5000,
		TIMER_VALUE_LOOP_PAUSE_DELAY = 1000
	};

//-------------------------------------------------------------------

void timerInit();
void timerSet(char timer_name, unsigned int start_value, unsigned int stop_value);
char timerIsElapsed(char timer_name);
unsigned int timerGetCurrent(char timer_name);



#endif
