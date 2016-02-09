#include "nock_machine.h"
#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "softtimer.h"
#include "eepromworker.h"
//=============================================================================
volatile
char g_flags[FLAG_TOTAL_COUNT];
//=============================================================================
volatile
Nock g_nocks[NOCK_G_NOCK_TOTAL_COUNT];
//=============================================================================
//inline
void initGVals(){
	loggerWriteToMarker((LogMesT)" initGVals() \n*", '*');
	char i, j;
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
void changeFlag(char name, char state){
	//loggerWriteToMarker((LogMesT)" changing flags \n*", '*');
	g_flags[name] = state;
}
//=============================================================================
char processWaitNoise();
char processCheckNock();
//=============================================================================
void nockReadFromEEPROM(Nock * nock);
void nockWriteToEEPROM(Nock * nock);
//=============================================================================
void initNockMachine_0(){
	loggerWriteToMarker((LogMesT)" initNockMachine_0() \n*", '*');
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
			loggerWriteToMarker((LogMesT)" goto WAIT_NOISE \n*", '*');
			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case WAIT_NOISE:
		//=========================
			if (processWaitNoise() == TRUE){
				loggerWriteToMarker((LogMesT)" goto CHECK_NOCK \n*", '*');
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
				loggerWriteToMarker((LogMesT)" goto PROCESS_ERROR \n*", '*');
				state = PROCESS_ERROR;
				break;
			}

			if (g_flags[FLAG_NOCK] == FALSE)	return;

			g_nocks[NOCK_CURRENT].nock[g_nocks[NOCK_CURRENT].count++] = timerGetCurrent(TIMER_NOCK);
			timerSet(TIMER_NOCK, TIMER_VALUE_INIT_ZERRO, TIMER_VALUE_MAX);

			loggerWriteToMarker((LogMesT)" goto WAIT_NOISE \n*", '*');
			state = WAIT_NOISE;
		break;
		//================================================
		//=========================
		case CHECK_NOCK:
		//=========================
			if (g_flags[FLAG_NEW_NOCK] == TRUE){ // we recording new nock
				loggerWriteToMarker((LogMesT)" goto WAIT_CURRENT_NOCK \n*", '*');
				state = WAIT_CURRENT_NOCK;
				break;
			}

			if (g_nocks[NOCK_CURRENT].count == g_nocks[NOCK_PATTERN].count){
				if (processCheckNock() == TRUE){
					loggerWriteToMarker((LogMesT)" goto PROCESS_OK \n*", '*');
					state = PROCESS_OK;
				}else{
					loggerWriteToMarker((LogMesT)" goto PROCESS_ERROR \n*", '*');
					state = PROCESS_ERROR;
				}
			}else{
				loggerWriteToMarker((LogMesT)" goto WAIT_CURRENT_NOCK \n*", '*');
				state = WAIT_CURRENT_NOCK;
			}
		break;
		//================================================
		//=========================
		case PROCESS_OK:
		//=========================
			loggerWriteToMarker((LogMesT)" in PROCESS_OK \n*", '*');
			g_flags[FLAG_NOCK_CORRECT] = TRUE;
			loggerWriteToMarker((LogMesT)" goto PROCESS_RESSET \n*", '*');
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_ERROR:
		//=========================
			loggerWriteToMarker((LogMesT)" in PROCESS_ERROR \n*", '*');
			g_flags[FLAG_NOCK_CORRECT] = FALSE;
			loggerWriteToMarker((LogMesT)" goto PROCESS_RESSET \n*", '*');
			state = PROCESS_RESSET;
		break;
		//================================================
		//=========================
		case PROCESS_RESSET:
		//=========================
			loggerWriteToMarker((LogMesT)" in PROCESS_RESSET \n*", '*');
			timerSet(TIMER_NOCK_TIMEOUT, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			timerSet(TIMER_NOCK, TIMER_VALUE_MAX, TIMER_VALUE_MAX);
			g_nocks[NOCK_CURRENT].count = NOCK_NO_NOCK;
			loggerWriteToMarker((LogMesT)" goto WAIT_FIRST_NOCK \n*", '*');
			state = WAIT_FIRST_NOCK;
		break;
		//================================================
		//=========================
		case PROCESS_NEW_NOCK:
		//=========================
			loggerWriteToMarker((LogMesT)" in PROCESS_NEW_NOCK \n*", '*');
			//writeNockToEEPROM(&g_nocks[NOCK_CURRENT]);
			g_nocks[NOCK_PATTERN] = g_nocks[NOCK_CURRENT];
			g_flags[FLAG_NEW_NOCK] = FALSE;
			loggerWriteToMarker((LogMesT)" goto PROCESS_RESSET \n*", '*');
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
			loggerWriteToMarker((LogMesT)" in processWaitNoise() first \n*", '*');
			timerSet(TIMER_NOCK_NOISE, 0, TIMER_VALUE_NOISE); // run nock noise timer
			local_state = SECOND;
		break;

		case SECOND:
			if (timerIsElapsed(TIMER_NOCK_NOISE) != TRUE) return FALSE;
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
	loggerWriteToMarker((LogMesT)" in processCheckNock() \n*", '*');
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
void nockReadFromEEPROM(Nock * nock){
	loggerWriteToMarker((LogMesT)" in nockReadFromEEPROM() \n*", '*');
	eepromRead(&(nock->count), 1, EEPROM_NOCK_START_ADDRESS);
	eepromRead((char *)nock->nock, nock->count * 2, EEPROM_NOCK_START_ADDRESS + 1);
}
//=======================================================================================
//inline
void nockWriteToEEPROM(Nock * nock){
	loggerWriteToMarker((LogMesT)" in nockWriteToEEPROM() \n*", '*');
	eepromWrite(&(nock->count), 1, EEPROM_NOCK_START_ADDRESS);
	eepromWrite((char *)nock->nock, nock->count * 2, EEPROM_NOCK_START_ADDRESS + 1);
}
//=======================================================================================
//=======================================================================================
//========   COMMANDS     COMMANDS     COMMANDS      COMMANDS             ===============
//=======================================================================================
//=======================================================================================
//inline
void userCommandMachine_1(){
	enum {WAIT_ACTION, NEW_NOCK_CODE_BUTTON_PRESSED};
	static char state = WAIT_ACTION;
	switch(state){
		case WAIT_ACTION:
			if (buttonIsPressed(BUTOONNEWCODE)){
				changeFlag(FLAG_NOCK, TRUE); break; //DEBUG
				state = NEW_NOCK_CODE_BUTTON_PRESSED;
				g_flags[FLAG_NEW_NOCK] = TRUE;
				loggerWriteToMarker((LogMesT)" in userCommandMachine_1() NEW_NOCK_CODE_BUTTON_PRESSED \n*", '*');
			}
		break;

		case NEW_NOCK_CODE_BUTTON_PRESSED:
			if (g_flags[FLAG_NEW_NOCK] == FALSE){
				loggerWriteToMarker((LogMesT)" in userCommandMachine_1() g_flags[FLAG_NEW_NOCK] == FALSE \n*", '*');
				state = WAIT_ACTION;
			}
		break;

		default:
		break;
	}
}
//=======================================================================================
//=======================================================================================
//===========         RADIO    RADIO    RADIO    RADIO    RADIO    ======================
//=======================================================================================
//=======================================================================================
char processWaitRadio();
//=======================================================================================
//inline
void radioSendMachine_3(){
	enum {	DATAMAXLEN = 64,
			DATAMARKERPOSITION = 0, DATAMARKER = '*',
			DATAIDPOSITION = 1, 	DATAID = 0b11011011,
			DATACODELENPOSITION = 2,
			DATACODEPOSION = 3		};
	static char data[DATAMAXLEN];


	enum {RADIOWAITNEWCODE, RADIOWAITCODEDONE, RADIOWAITBOX, RADIOSENDTOBOX, RADIOWAITDELAY};
	static char state = RADIOWAITNEWCODE;

	switch (state){
		//========================
		case RADIOWAITNEWCODE:
		//========================
			if (g_flags[FLAG_NEW_NOCK] == TRUE)
				state = RADIOWAITCODEDONE;
		break;
		//========================
		case RADIOWAITCODEDONE:
		//========================
			if (g_flags[FLAG_NEW_NOCK] == FALSE)
				state = RADIOWAITBOX;
		break;
		//========================
		case RADIOWAITBOX:{
		//========================
			int len = 0;//rfmReadData(data);
			if (len > 0){
				if (data[DATAIDPOSITION] == DATAID){
					state = RADIOSENDTOBOX;
					break;
				}
			}
			//rfmToTxMode();

			enum {ASKMESLEN = 3};
			char mes[ASKMESLEN] = {DATAMARKER, DATAID, DATAID};

			//rfmSend(mes, ASKMESLEN);
			//rfmToRxMode();

			state = RADIOWAITDELAY;
		}break;
		//========================
		case RADIOWAITDELAY:
		//========================
			if (processWaitRadio() == TRUE)
				state = RADIOWAITBOX;
		break;
		//========================
		case RADIOSENDTOBOX:{
		//========================
			//rfmToTxMode();

			data[DATAMARKERPOSITION] = DATAMARKER;
			data[DATAIDPOSITION] = DATAID;
			data[DATACODELENPOSITION] = g_nocks[NOCK_CURRENT].count;

			char i;
			for (i = 0; g_nocks[NOCK_CURRENT].count; i++){
				data[DATACODEPOSION + i] = g_nocks[NOCK_CURRENT].nock[i];
			}

			//rfmSend(data, DATACODEPOSION + g_nocks[NOCK_CURRENT].count + 1);
			state = RADIOWAITNEWCODE;
		}break;
		//========================
		default: break;
		//========================
	}
}
//=======================================================================================
char processWaitRadio(){
	enum {FIRST, SECOND};
	static char local_state = FIRST;

	switch (local_state){
		case FIRST:
			timerSet(TIMER_RADIO_WAIT, 0, TIMER_VALUE_RADIO_WAIT); // run nock noise timer
			local_state = SECOND;
		break;

		case SECOND:
			if (timerIsElapsed(TIMER_RADIO_WAIT) != TRUE) return;
			local_state = FIRST;
			return TRUE;
		break;

		default: break;
	}
	return FALSE;
}
//=======================================================================================
