#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "leds.h"
#include "softtimer.h"
#include "eepromworker.h"
#include "nock_machine.h"
#include "send_machine.h"
#include "colibrator.h"

#include <avr/io.h>

#include <avr/delay.h>

void initMyExtIRQ();

#define LOGGDEBUG

int main(void)
{
	

	ledInit();
	buttonsInit();
	loggerInit();
	timerInit();
	initMyExtIRQ();

	if(buttonIsPressed(BUTTONNEWNOCK) == TRUE){
		#ifdef LOGGDEBUG
		loggerWriteToMarker((LogMesT)"\r\r\r goto colibrateDetector \r*", '*');
		#endif

		while(1)
			colibrateDetector();
	}

	#ifdef LOGGDEBUG
	loggerWriteToMarker((LogMesT)"\r\r\r Starting program \r*", '*');
	#endif
		
	initNockMachine();

	#ifdef LOGGDEBUG
	loggerWriteToMarker((LogMesT)" iRaSeMac \r*", '*');
	#endif

	initSendMachine();

	#ifdef LOGGDEBUG
	loggerWriteToMarker((LogMesT)" exit iRaSeMac \r*", '*');
	#endif

//	ledOn(LEDRED2);
//timerSet(TIMER_NOCK,0,50);

   while(1)
    {

		//if(timerIsElapsed(TIMER_NOCK) == TRUE){
		//	timerSet(TIMER_NOCK,0,50);
		//	ledTaggle(LEDRED1);
		//}
		//continue;
		
    	userNewNockCommandMachine_1();
    	nockMachine_2();
    	sendMachine_3();
		doorSignalMachine_4();

    }
}
