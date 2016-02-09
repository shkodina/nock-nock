#include "nock_machine.h"
#include "send_machine.h"
#include "common_makros.h"

#include "buttons.h"
#include "softtimer.h"
#include "leds.h"
#include "eepromworker.h"

//#define LOGG
#define LOGGRES
//#define LOGGBUT

//#ifdef LOGG 
#include "uart_logger.h"
//#endif

//=============================================================================
volatile
char g_flags[FLAG_TOTAL_COUNT];
//=============================================================================
volatile
Nock g_nocks[NOCK_G_NOCK_TOTAL_COUNT];
//=============================================================================
Nock * getNock(char name){
	return &g_nocks[name];
}
//=============================================================================
//inline
void initGVals(){
	#ifdef LOGG	
	loggerWriteToMarker((LogMesT)" initGVals() \r*", '*');
	#endif
	unsigned char i, j;
	// =======================================
	for (j = 0; j < NOCK_G_NOCK_TOTAL_COUNT; j++){
		g_nocks[j].count = NOCK_NO_NOCK;
		for (i = 0; i < NOCK_MAX_COUNT; i++){
			g_nocks[j].nock[i] = NOCK_NO_NOCK;
		}
	}
	//========================================
	for (i = 0; i < FLAG_TOTAL_COUNT; i++){
		g_flags[i] = FALSE;
	}
}
//=============================================================================
void changeFlag(unsigned char name, char state){
	//loggerWriteToMarker((LogMesT)" changing flags \r*", '*');
	g_flags[(unsigned char)name] = state;
}
//=============================================================================
char getFlag(unsigned char name){
	return g_flags[(unsigned char)name];
}
//=============================================================================
char processWaitNoise();
char processCheckNock();
//=============================================================================
void nockReadFromEEPROM(Nock * nock);
void nockWriteToEEPROM(Nock * nock);
//=============================================================================
void initNockMachine(){
	initGVals();
	nockReadFromEEPROM(&g_nocks[NOCK_PATTERN]);
}
//=============================================================================
void nockMachine_2(){
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
			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" goto WAIT_NOISE \r*", '*');
			#endif
			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case WAIT_NOISE:
		//=========================
			if (processWaitNoise() == TRUE){
				#ifdef LOGG	
				loggerWriteToMarker((LogMesT)" goto CHECK_NOCK \r*", '*');
				#endif
				state = CHECK_NOCK;
			}
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
				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" goto PROCESS_ERROR \r*", '*');
				#endif
				state = PROCESS_ERROR;
				break;
			}

			if (g_flags[FLAG_NOCK] == FALSE)	return;

			g_nocks[NOCK_CURRENT].nock[(unsigned char)g_nocks[NOCK_CURRENT].count++] = timerGetCurrent(TIMER_NOCK);
			timerSet(TIMER_NOCK, TIMER_VALUE_INIT_ZERRO, TIMER_VALUE_MAX);
			#ifdef LOGG
			loggerWriteToMarker((LogMesT)" goto WAIT_NOISE \r*", '*');
			#endif
			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case CHECK_NOCK:
		//=========================
			if (g_flags[FLAG_NEW_NOCK] == TRUE){ // we recording new nock
				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" goto WAIT_CURRENT_NOCK \r*", '*');
				#endif
				state = WAIT_CURRENT_NOCK;
				break;
			}


			if (g_nocks[NOCK_CURRENT].count == g_nocks[NOCK_PATTERN].count){
				if (processCheckNock() == TRUE){
					#ifdef LOGG
					loggerWriteToMarker((LogMesT)" goto PROCESS_OK \r*", '*');
					#endif
					state = PROCESS_OK;
				}else{
					#ifdef LOGG
					loggerWriteToMarker((LogMesT)" goto PROCESS_ERROR \r*", '*');
					#endif
					state = PROCESS_ERROR;
				}
			}else{
				char pos = g_nocks[NOCK_CURRENT].count - 1;
				if (g_nocks[NOCK_CURRENT].nock[pos] > g_nocks[NOCK_PATTERN].nock[pos] + NOCK_DELTA
					||
					g_nocks[NOCK_CURRENT].nock[pos] < g_nocks[NOCK_PATTERN].nock[pos] - NOCK_DELTA)
				{
					#ifdef LOGG
					loggerWriteToMarker((LogMesT)" goto PROCESS_ERROR \r*", '*');
					#endif
					state = PROCESS_ERROR;
				}

				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" goto WAIT_CURRENT_NOCK \r*", '*');
				#endif
				state = WAIT_CURRENT_NOCK;
			}
		break;
		//================================================
		//=========================
		case PROCESS_OK:
		//=========================
			#ifdef LOGGRES	
			loggerWriteToMarker((LogMesT)" in PROCESS_OK \r*", '*');
			#endif
			g_flags[FLAG_NOCK_CORRECT] = TRUE;
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_ERROR:
		//=========================
			#ifdef LOGGRES	
			loggerWriteToMarker((LogMesT)" in PROCESS_ERROR \r*", '*');
			#endif
			g_flags[FLAG_NOCK_CORRECT] = FALSE;
			g_flags[FLAG_ERROR] = TRUE;
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_RESSET:
		//=========================
			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" in PROCESS_RESSET \r*", '*');
			#endif
			timerSet(TIMER_NOCK_TIMEOUT, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			timerSet(TIMER_NOCK, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			g_nocks[NOCK_CURRENT].count = NOCK_NO_NOCK;
			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" goto WAIT_FIRST_NOCK \r*", '*');
			#endif
			state = WAIT_FIRST_NOCK;
		break;
		//================================================
		//=========================
		case PROCESS_NEW_NOCK:
		//=========================

			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" in PROCESS_NEW_NOCK \r*", '*');
			#endif
			nockWriteToEEPROM(&g_nocks[NOCK_CURRENT]);
			g_nocks[NOCK_PATTERN] = g_nocks[NOCK_CURRENT];
			g_flags[FLAG_NEW_NOCK] = FALSE;
			#ifdef LOGG
			loggerWriteToMarker((LogMesT)" goto PROCESS_RESSET \r*", '*');
			#endif
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
	enum {FIRST, SECOND};
	static char local_state = FIRST;

	switch (local_state){
		case FIRST:
			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" in processWaitNoise() first \r*", '*');
			#endif
			timerSet(TIMER_NOCK_NOISE, 0, TIMER_VALUE_NOISE); // run nock noise timer
			local_state = SECOND;

			ledTaggle(LEDGREEN1);
			ledTaggle(LEDGREEN2);
		break;

		case SECOND:
			if (timerIsElapsed(TIMER_NOCK_NOISE) != TRUE) return FALSE;
			g_flags[FLAG_NOCK] = FALSE; // clear flag if it was noise
			local_state = FIRST;

			ledTaggle(LEDGREEN1);
			ledTaggle(LEDGREEN2);

			return TRUE;
		break;

		default: break;
	}
	return FALSE;
}
//=======================================================================================
//inline
char processCheckNock(){
	#ifdef LOGG	
	loggerWriteToMarker((LogMesT)" in processCheckNock() \r*", '*');
	#endif
	unsigned char i;
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
void nockReadFromEEPROM(Nock * nock){
	#ifdef LOGG
	loggerWriteToMarker((LogMesT)" in nockReadFromEEPROM\r*", '*'); 
	#endif
	eepromRead(&(nock->count), 1, EEPROM_NOCK_START_ADDRESS);
	if (nock->count < 0 || nock->count > NOCK_MAX_COUNT){
		#ifdef LOGG
		loggerWriteToMarker((LogMesT)" no nock in eeprom\r*", '*'); 
		#endif
		nock->count = 0;
		return;
	}
	eepromRead((char *)nock->nock, nock->count * 2, EEPROM_NOCK_START_ADDRESS + 1);
}
//=======================================================================================
//inline
void nockWriteToEEPROM(Nock * nock){
	#ifdef LOGG
	loggerWriteToMarker((LogMesT)" in nockWriteToEEPROM\r*", '*'); 
	#endif
	eepromWrite(&(nock->count), 1, EEPROM_NOCK_START_ADDRESS);
	eepromWrite((char *)nock->nock, nock->count * 2, EEPROM_NOCK_START_ADDRESS + 1);
}
//=======================================================================================
//=======================================================================================
//========   COMMANDS     COMMANDS     COMMANDS      COMMANDS             ===============
//=======================================================================================
//=======================================================================================
//inline
void userNewNockCommandMachine_1(){
	enum {WAIT_ACTION, NEW_NOCK_CODE_BUTTON_PRESSED};
	static char state = WAIT_ACTION;
	switch(state){
		case WAIT_ACTION:
			if (buttonIsPressed(BUTTONNEWNOCK)  == TRUE){
				state = NEW_NOCK_CODE_BUTTON_PRESSED;
				g_flags[FLAG_NEW_NOCK] = TRUE;
				#ifdef LOGGBUT		
				loggerWriteToMarker((LogMesT)" in userCommandMachine_1() NEW_NOCK_CODE_BUTTON_PRESSED \r*", '*'); 
				#endif

				ledOn(LEDRED1);
				ledOn(LEDRED2);
				ledOn(LEDGREEN1);
				ledOn(LEDGREEN2);
			}
			break;

		case NEW_NOCK_CODE_BUTTON_PRESSED:
			if (g_flags[FLAG_NEW_NOCK] == FALSE){
				#ifdef LOGGBUT 
				loggerWriteToMarker((LogMesT)" in userCommandMachine_1() g_flags[FLAG_NEW_NOCK] == FALSE \r*", '*'); 
				#endif
				state = WAIT_ACTION;

				ledOff(LEDGREEN1);
				ledOff(LEDGREEN2);
			}



			break;

		default:
		break;
	}
}

//=======================================================================================
//=======================================================================================
//=======================================================================================
//===         DOOR and SIGNALS      DOOR and SIGNALS      DOOR and SIGNALS      =========
//=======================================================================================
//=======================================================================================
void doorSignalMachine_4(){

	if (g_flags[FLAG_ERROR] == TRUE){
		#ifdef LOGGBUT		
		loggerWriteToMarker((LogMesT)" in error\r*", '*'); 
		#endif
		g_flags[FLAG_ERROR] = FALSE;
		timerSet(TIMER_ERROR, 0, TIMER_VALUE_ERROR);
		while(timerIsElapsed(TIMER_ERROR) != TRUE){
			ledOff(LEDGREEN1);
			ledOff(LEDRED1);
			ledOff(LEDGREEN2);
			ledOff(LEDRED2);		
		}
		#ifdef LOGGBUT		
		loggerWriteToMarker((LogMesT)" exit error\r*", '*'); 
		#endif
		ledOn(LEDRED1);
		ledOn(LEDRED2);		

	}

#define BUTTONWAITDELAY 30

	enum DOOR {DOORISOPEN, DOORISCLOSE, DOORNEEDOPEN, DOORNEEDCLOSE};
	static char state_door = DOORNEEDCLOSE;
	switch (state_door){
		//========================
		case DOORISCLOSE:
		//========================
			if (buttonIsPressed(BUTTONDOOR)  == TRUE){
				if (timerGetCurrent(TIMER_DOOR_OPEN) < BUTTONWAITDELAY) 
					return;
				g_flags[FLAG_NOCK_CORRECT] = TRUE;
			}

			if (g_flags[FLAG_NOCK_CORRECT] == TRUE)
				state_door = DOORNEEDOPEN;

			break;
		//========================
		case DOORNEEDOPEN:
		//========================
			ledOn(LEDGREEN1);
			ledOff(LEDRED1);
			ledOn(LEDGREEN2);
			ledOff(LEDRED2);

			#ifdef LOGGBUT		
			loggerWriteToMarker((LogMesT)" DOORNEEDOPEN\r*", '*'); 
			#endif
			
			state_door = DOORISOPEN;

			timerSet(TIMER_DOOR_OPEN, 0, TIMER_VALUE_DOOR_OPEN);
			break;
		//========================
		case DOORISOPEN:
		//========================
			if (timerIsElapsed(TIMER_DOOR_OPEN) == TRUE)
				g_flags[FLAG_NOCK_CORRECT] = FALSE;
			
			if (buttonIsPressed(BUTTONDOOR) == TRUE){

				if (timerGetCurrent(TIMER_DOOR_OPEN) < BUTTONWAITDELAY) 
					return;

				g_flags[FLAG_NOCK_CORRECT] = FALSE;
			}

			if (g_flags[FLAG_NOCK_CORRECT] == FALSE)
				state_door = DOORNEEDCLOSE;

			break;
		//========================
		case DOORNEEDCLOSE:
		//========================
			ledOn(LEDRED1);
			ledOff(LEDGREEN1);
			ledOn(LEDRED2);
			ledOff(LEDGREEN2);

			#ifdef LOGGBUT		
			loggerWriteToMarker((LogMesT)" DOORNEEDCLOSE\r*", '*'); 
			#endif	
					
			state_door = DOORISCLOSE;
			timerSet(TIMER_DOOR_OPEN, 0, TIMER_VALUE_DOOR_OPEN);
			break;
		//========================
		default: break;
		//========================
	}
}
