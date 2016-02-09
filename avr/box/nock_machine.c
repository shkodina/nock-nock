#include "nock_machine.h"
#include "common_makros.h"

#include "softtimer.h"
#include "leds.h"
#include "eepromworker.h"
#include "rf73_spi.h"

#include <avr/io.h>
#include <avr/delay.h>


#define LOGG
#define LOGGF
#ifdef LOGGF
#include "uart_logger.h"
#endif

#define DEBUG

//=============================================================================
void makeNock(){
	for(char i = 0; i < 100; i++){
		INVBIT(PORTD, 3);
		_delay_ms(3);
	}
}
//=============================================================================
volatile
char g_flags[FLAG_TOTAL_COUNT];
//=============================================================================
volatile
Nock g_nocks[NOCK_G_NOCK_TOTAL_COUNT];
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
	g_flags[(unsigned char)name] = state;
}
//=============================================================================
void nockReadFromEEPROM(Nock * nock);
void nockWriteToEEPROM(Nock * nock);
//=============================================================================
void initNockMachine(){
	DDRD |= (1 << 3);
	#ifdef LOGG	
	loggerWriteToMarker((LogMesT)" initNockMachine_0() \r*", '*');
	#endif
	initGVals();
	nockReadFromEEPROM(&g_nocks[NOCK_PATTERN]);
}
//=============================================================================
void nockMachine_2(){
	enum {RESSET,PLAY_NOCK, TIMER_NOCK_DELAY, NEXT_LOOP, NEXT_LOOP_PAUSE};
	static char state = RESSET;
	static unsigned char nock_pos = 0;

	if (g_flags[FLAG_NEW_NOCK] == TRUE){
		g_flags[FLAG_NEW_NOCK] = FALSE;
		state = NEXT_LOOP;

	}



	switch (state){
		//=========================
		case RESSET:
		//=========================
			nock_pos = 0;
			state = PLAY_NOCK;
			break;
		//=========================
		case PLAY_NOCK:
		//=========================
			if (nock_pos == g_nocks[NOCK_PATTERN].count){
				makeNock();
				state = NEXT_LOOP;
				break;
			}
				
			makeNock();
			timerSet(TIMER_NOCK, 0, g_nocks[NOCK_PATTERN].nock[nock_pos++]);
			state = TIMER_NOCK_DELAY;
			break;
		//=========================
		case TIMER_NOCK_DELAY:
		//=========================
			if (timerIsElapsed(TIMER_NOCK) == TRUE){
				state = PLAY_NOCK;
			}
			break;
		//=========================
		case NEXT_LOOP:
		//=========================
			timerSet(TIMER_NOCK, 0, TIMER_VALUE_LOOP_PAUSE_DELAY);
			state = NEXT_LOOP_PAUSE;
			break;
		//=========================
		case NEXT_LOOP_PAUSE:
		//=========================
			if (timerIsElapsed(TIMER_NOCK) == TRUE){
				state = RESSET;
			}
			break;
		//================================================
		default:
		break;
	}
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

		
	#ifdef DEBUG
	nock->nock[0] = 50;
	nock->nock[1] = 50;
	nock->nock[2] = 50;

	nock->count = 3;
	return;
	#endif

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
//=========================================================================================================
//==================================================================================================================
//===========         RADIO    RADIO    RADIO    RADIO    RADIO    ==========================================================
//==================================================================================================================
//=========================================================================================================
//#include "rf73_spi.h"

char processWaitSender();
void initSendMachine();
//=======================================================================================
void initSendMachine(){
	//senderInit();
	Init_Spi();
	RFM73_Initialize();
	RFM73_SetPower(0x03);
	SwitchToRxMode();
}
//=======================================================================================
//inline
void sendMachine_3(){
	enum {	DATAMAXLEN = 64,
			DATAMARKERPOSITION = 0, DATAMARKER = '?',
			DATACODELENPOSITION = 1,
			DATACODEPOSION = 2		};

	#define ASKDATALEN 3
	char ASKDATA[ASKDATALEN] = "***";
	#define ANSWERDATALEN 3
	char ANSWERDATA[ANSWERDATALEN] = "+++";

	static char data[DATAMAXLEN];


	enum {INITWAITNEWCODE, WAITDOOR, ANSWERDOOR, WAITCODE};
	static char state = INITWAITNEWCODE;

	switch (state){
		//========================
		case INITWAITNEWCODE:
		//========================
			#ifdef LOGG
			loggerWriteToMarker((LogMesT)" INITWAITNEWCODE\r*", '*'); 
			#endif

			SwitchToRxMode();
			state = WAITDOOR;

			timerSet(TIMER_RADIO_WAIT, 0, 50);
			break;
		//========================
		case WAITDOOR:{
		//========================
			if (timerIsElapsed(TIMER_RADIO_WAIT) == FALSE)
				break;

			int len = Receive_Packet(data, DATAMAXLEN);
			if (len > 0){

				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" get some data*", '*'); 
				loggerWrite(data, len);
				loggerWrite("\r", 1);
				#endif

				for (char ff = 0; ff < 10; ff++){
					makeNock();
					_delay_ms(200);
				}
				/* //DEBUG
				for (char i = 0; i < ASKDATALEN; i++)
					if (data[i] != ASKDATA[i]){
						timerSet(TIMER_RADIO_WAIT, 0, 50);
						return;
					}

				state = ANSWERDOOR; 
				*/

				char pos = 0;
				for (pos = 0; pos < len; pos++){
					if (data[pos] == DATAMARKER){
						break;
					}
				}
				
				if (pos >= len){ // no datamarker
					state = INITWAITNEWCODE;
					return;
				}
				pos++;
				char count = data[pos++];// - 48; //debug

				#ifdef LOGG
				char ch = count + 48;
				loggerWriteToMarker((LogMesT)" WAITCODE nock count: *", '*');
				loggerWrite(&ch, 1); 
				loggerWrite("\r", 1);
				#endif 


				g_nocks[NOCK_CURRENT].count = count;
				unsigned char i;
				for (i = 0; i < count; i++){
					int time = 0;
					char * pp = &time;
					*pp =  data[pos++];
					pp++;
					*pp =  data[pos++];


					g_nocks[NOCK_CURRENT].nock[i] = time;
				}

				g_flags[FLAG_NEW_NOCK] = TRUE;
				nockWriteToEEPROM(&g_nocks[NOCK_CURRENT]);
				g_nocks[NOCK_PATTERN] = g_nocks[NOCK_CURRENT];
				state = INITWAITNEWCODE;
			}
			timerSet(TIMER_RADIO_WAIT, 0, 50);
			}break;

		//========================
		case ANSWERDOOR:
		//========================

			#ifdef LOGG
			loggerWriteToMarker((LogMesT)" ANSWERDOOR\r*", '*'); 
			#endif

			SwitchToTxMode();

			Send_Packet(W_TX_PAYLOAD_NOACK_CMD,ANSWERDATA, ANSWERDATALEN );

			SwitchToRxMode();

			state = WAITCODE;

			timerSet(TIMER_RADIO_WAIT, 0, 150);

			break;
		//========================
		case WAITCODE:{
		//========================
			if (timerIsElapsed(TIMER_RADIO_WAIT) == FALSE)
				break;

			timerSet(TIMER_RADIO_WAIT, 0, 150);

			#ifdef LOGG
			loggerWriteToMarker((LogMesT)" WAITCODE\r*", '*'); 
			#endif

			int len = Receive_Packet(data, DATAMAXLEN);
			if (len > 0){

				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" WAITCODE get some data:*", '*'); 
				loggerWrite(data, len);
				loggerWrite("\r",1);
				#endif

				char pos = 0;
				for (pos = 0; pos < len; pos++){
					if (data[pos] == DATAMARKER){
						break;
					}
				}

				for (char ff = 0; ff < 10; ff++){
					makeNock();
					_delay_ms(200);
				}
				_delay_ms(500);
				for (char ff = 0; ff < 10; ff++){
					makeNock();
					_delay_ms(200);
				}

				if (pos >= len) // no owr data
					break;
				pos++;
				char count = data[pos++];// - 48; //debug

				#ifdef LOGG
				char ch = count + 48;
				loggerWriteToMarker((LogMesT)" WAITCODE nock count: *", '*');
				loggerWrite(&ch, 1); 
				loggerWrite("\r", 1);
				#endif 


				g_nocks[NOCK_CURRENT].count = count;
				unsigned char i;
				for (i = 0; i < count; i++){
					g_nocks[NOCK_CURRENT].nock[i] = data[pos++];
				}


				#ifdef LOGG
				loggerWriteToMarker((LogMesT)" WAITCODE finish copy \r*", '*'); 
				loggerWrite(data, len);
				loggerWrite("\r",1);
				#endif


				g_flags[FLAG_NEW_NOCK] = TRUE;
				nockWriteToEEPROM(&g_nocks[NOCK_CURRENT]);
				g_nocks[NOCK_PATTERN] = g_nocks[NOCK_CURRENT];
				state = INITWAITNEWCODE;
			}
			}break;
		//========================
		default: break;
		//========================
	}
}
//=======================================================================================
