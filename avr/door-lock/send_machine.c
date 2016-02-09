#include "rf73_spi.h"
#include "nock_machine.h"
#include "send_machine.h"
#include "common_makros.h"

#include "softtimer.h"


//#define LOGG

#ifdef LOGG
#include "uart_logger.h"
#endif


//=======================================================================================
//=======================================================================================
//===========         RADIO    RADIO    RADIO    RADIO    RADIO    ======================
//=======================================================================================
//=======================================================================================


char processWait();
void initSendMachine();
//=======================================================================================
void initSendMachine(){
	//senderInit();
	Init_Spi();
	RFM73_Initialize();
	RFM73_SetPower(0x03);
	SwitchToTxMode();
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


	enum {WAITNEWCODE, WAITCODEDONE, WAITBOX, SENDTOBOX, WAITDELAY};
	static char state = WAITNEWCODE;

	switch (state){
		//========================
		case WAITNEWCODE:
		//========================
			if (getFlag(FLAG_NEW_NOCK) == TRUE)
				state = WAITCODEDONE;
		break;
		//========================
		case WAITCODEDONE:
		//========================
			if (getFlag(FLAG_NEW_NOCK) == FALSE)
				state = WAITBOX;
		break;
		//========================
		case WAITBOX:{
		//========================
			char len = Receive_Packet(data, DATAMAXLEN);
			if (len > 0){

				#ifdef LOGG	
				loggerWriteToMarker((LogMesT)" WAITBOX get data: *", '*');
				loggerWrite((LogMesT)data, len);
				#endif

				for (char i = 0; i < ANSWERDATALEN; i++)
					if (data[i] != ANSWERDATA[i])
						return;
				
				state = SENDTOBOX;
				break;
			}
			SwitchToTxMode();

			Send_Packet(W_TX_PAYLOAD_NOACK_CMD, (unsigned char *)ASKDATA, ASKDATALEN);
			//Send_Packet(0, ASKDATA, ASKDATALEN);
			SwitchToRxMode();

			state = WAITDELAY;
		}break;
		//========================
		case WAITDELAY:
		//========================
			if (processWait() == TRUE)
				#ifdef LOGG	
				loggerWriteToMarker((LogMesT)" goto WAITBOX\r*", '*');
				#endif

				state = WAITBOX;
		break;
		//========================
		case SENDTOBOX:{
		//========================

			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" SENDTOBOX\r*", '*');
			#endif

			SwitchToTxMode();

			Nock * nock = getNock(NOCK_PATTERN);

			data[DATAMARKERPOSITION] = DATAMARKER;
			data[DATACODELENPOSITION] = nock->count;

			unsigned char i;
			for (i = 0; i < nock->count; i++){
				data[DATACODEPOSION + i] = nock->nock[i];
			}

			data[DATACODEPOSION + i] = DATAMARKER;
			Send_Packet(W_TX_PAYLOAD_NOACK_CMD, (unsigned char *)data, DATACODEPOSION + nock->count + 1);
			//Send_Packet(0, (unsigned char *)data, DATACODEPOSION + nock->count + 1);

			#ifdef LOGG	
			loggerWriteToMarker((LogMesT)" nock count: *", '*');
			char ch = nock->count + 48;
			loggerWrite(&ch, 1);
			#endif


			state = WAITNEWCODE;
		}break;
		//========================
		default: break;
		//========================
	}
}
//=======================================================================================
char processWait(){
	enum {FIRST, SECOND};
	static char local_state = FIRST;

	switch (local_state){
		case FIRST:
			timerSet(TIMER_RADIO_WAIT, 0, TIMER_VALUE_RADIO_WAIT); // run nock noise timer
			local_state = SECOND;
		break;

		case SECOND:
			if (timerIsElapsed(TIMER_RADIO_WAIT) != TRUE) return FALSE;
			local_state = FIRST;
			return TRUE;
		break;

		default: break;
	}
	return FALSE;
}
