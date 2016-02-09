#include "nock_machine.h"
#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "softtimer.h"
#include "eepromworker.h"
//=============================================================================
enum {	FLAG_TOTAL_COUNT = 5,
		FLAG_NOCK = 0,
		FLAG_NOCK_CORRECT = 1,
		FLAG_NEW_NOCK = 2};
//volatile
char g_flags[FLAG_TOTAL_COUNT];
//=============================================================================
enum {	NOCK_MAX_COUNT = 30,
		NOCK_NO_NOCK = 0,
		NOCK_EEPROM_START_ADDRESS = 1,
		NOCK_DELTA = 5};
typedef struct {
	int nock[NOCK_MAX_COUNT];
	char count;
} Nock;
enum {	NOCK_G_NOCK_TOTAL_COUNT = 3,
		NOCK_PATTERN = 0,
		NOCK_CURRENT = 1,
		NOCK_NEW = 2};
//volatile
Nock g_nocks[NOCK_G_NOCK_TOTAL_COUNT];
//=============================================================================
char processWaitNoise();
char processCheckNock();
//=============================================================================
void nockMachine(){
	enum {WAIT_FIRST_NOCK, WAIT_NOISE, WAIT_CURRENT_NOCK, CHECK_NOCK, PROCESS_ERROR, PROCESS_OK, PROCESS_RESSET, PROCESS_NEW_NOCK};
	static char state = WAIT_FIRST_NOCK;

	switch (state){
		//================================================
		//=========================
		case WAIT_FIRST_NOCK:
		//=========================
			if (g_flags[FLAG_NOCK] == FALSE)	return;
			timerSet(TIMER_NOCK_TIMEOUT,  TIMER_VALUE_INIT_ZERRO, TIMER_VALUE_NOCK_TIMEOUT);
			timerSet(TIMER_NOCK, TIMER_VALUE_INIT_ZERRO, TIMER_VALUE_MAX);
			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case WAIT_NOISE:
		//=========================
			if (processWaitNoise() == TRUE)
				state = CHECK_NOCK;
		break;
		//================================================
		//=========================
		case WAIT_CURRENT_NOCK:
		//=========================
			if (timerIsElapsed(TIMER_NOCK_TIMEOUT) == TRUE){ // its timeout
				if (g_flags[FLAG_NEW_NOCK] == TRUE){
					state = PROCESS_NEW_NOCK;
					break;
				}

				state = PROCESS_ERROR;
				break;
			}

			if (g_flags[FLAG_NOCK] == FALSE)	return;

			g_nocks[NOCK_CURRENT].nock[g_nocks[NOCK_CURRENT].count++] = timerGetCurrent(TIMER_NOCK);
			timerSet(TIMER_NOCK, TIMER_VALUE_INIT_ZERRO, TIMER_VALUE_MAX);

			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case CHECK_NOCK:
		//=========================
			if (g_flags[FLAG_NEW_NOCK] == TRUE){ // we recording new nock
				state = WAIT_CURRENT_NOCK;
				break;
			}

			if (g_nocks[NOCK_CURRENT].count == g_nocks[NOCK_PATTERN].count){
				if (processCheckNock() == TRUE){
					state = PROCESS_OK;
				}else{
					state = PROCESS_ERROR;
				}
			}else{
				state = WAIT_CURRENT_NOCK;
			}
		break;
		//================================================
		//=========================
		case PROCESS_OK:
		//=========================
			g_flags[FLAG_NOCK_CORRECT] = TRUE;
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_ERROR:
		//=========================
			g_flags[FLAG_NOCK_CORRECT] = FALSE;
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_RESSET:
		//=========================
			timerSet(TIMER_NOCK_TIMEOUT, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			timerSet(TIMER_NOCK, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			g_nocks[NOCK_CURRENT].count = NOCK_NO_NOCK;
			state = WAIT_FIRST_NOCK;
		break;
		//================================================
		//=========================
		case PROCESS_NEW_NOCK:
		//=========================
			//writeNockToEEPROM(&g_nocks[NOCK_CURRENT]);
			g_nocks[NOCK_PATTERN] = g_nocks[NOCK_CURRENT];
			g_flags[FLAG_NEW_NOCK] == FALSE;
			state = PROCESS_RESSET;
		break;
		//================================================
		default:
		break;
	}
}
//=======================================================================================
//inline
char processWaitNoise(){
	enum {FIRST, NOTFIRST};
	static char local_state = FIRST;

	switch (local_state){
		case FIRST:
			setTimer(TIMER_NOCK_NOISE, 0, TIMER_VALUE_NOISE); // run nock noise timer
			local_state = NOTFIRST;
		break;

		case NOTFIRST:
			if (timerIsElapsed(TIMER_NOCK_NOISE) != TRUE) return;
			g_flags[FLAG_NOCK] = FALSE; // clear flag if it was noise
			local_state = FIRST;
			return TRUE;
		break;

		default: break;
	}
	return FALSE;
}
//=======================================================================================
//inline
char processCheckNock(){
	char i;
	for (i = 0; i < g_nocks[NOCK_CURRENT].count; i++){
		if (g_nocks[NOCK_CURRENT].nock[i] < g_nocks[NOCK_PATTERN].nock[i] + NOCK_DELTA
			&&
			g_nocks[NOCK_CURRENT].nock[i] > g_nocks[NOCK_PATTERN].nock[i] - NOCK_DELTA)
		{
			continue;
		}else{
			return FALSE;
		}
	}
	return TRUE;
}
//=======================================================================================
//inline
