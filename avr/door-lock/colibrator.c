#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "leds.h"
#include "softtimer.h"
#include "eepromworker.h"
#include "nock_machine.h"
#include "colibrator.h"

#include <avr/delay.h>

//#define LOGGDEBUG

void colibrateDetector(){

	enum {GOTOPAUSE, PAUSE, CHECKEXT, PROCESSBUTTON, PROCESSNOCK};
	static char state = GOTOPAUSE;
	
	switch (state){

		case GOTOPAUSE:

			#ifdef LOGGDEBUG
			loggerWriteToMarker((LogMesT)" GOTOPAUSE \r*", '*');
			#endif

			ledOff(LEDRED1);
			ledOff(LEDRED2);
			ledOff(LEDGREEN1);
			ledOff(LEDGREEN2);

			timerSet(TIMER_NOCK, 0, 30);
			state = PAUSE;
			break;

		case PAUSE:

			if (timerIsElapsed(TIMER_NOCK) == TRUE){
				state = CHECKEXT;
			}
			break;

		case CHECKEXT:

			if(buttonIsPressed(BUTTONDOOR) == TRUE){

				#ifdef LOGGDEBUG
				loggerWriteToMarker((LogMesT)" setNextSence \r*", '*');
				#endif

				setNextSence();
				changeFlag(FLAG_NOCK, FALSE);

				ledOn(LEDRED1);
				ledOn(LEDRED2);
				_delay_ms(300);
				ledOff(LEDRED1);
				ledOff(LEDRED2);


				state = GOTOPAUSE;
				return;
			}

			if (getFlag(FLAG_NOCK) == TRUE){
				state = PROCESSNOCK;
			}
				
			break;

		case PROCESSNOCK:
			
			#ifdef LOGGDEBUG
			loggerWriteToMarker((LogMesT)" PROCESSNOCK \r*", '*');
			#endif

			ledOn(LEDGREEN1);
			ledOn(LEDGREEN2);
			_delay_ms(300);
			ledOff(LEDGREEN1);
			ledOff(LEDGREEN2);

					
			changeFlag(FLAG_NOCK, FALSE);
			state = CHECKEXT;

			break;

		default: break;
	} 

}
