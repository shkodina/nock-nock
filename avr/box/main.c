#include "common_makros.h"

#include "softtimer.h"
#include "eepromworker.h"
#include "rf73_spi.h"

#include <avr/io.h>
#include <avr/delay.h>

#define LOGG
#ifdef LOGG
#include "uart_logger.h"
#endif

void initMyExtIRQ();

int main(void)
{

	eepromInit();
	timerInit();



	#ifdef LOGG
	loggerInit();
	loggerWriteToMarker((LogMesT)"\r\r Starting program \r*", '*');
	#endif

	initNockMachine();
	initSendMachine();

	#ifdef LOGG
	loggerWriteToMarker((LogMesT)" start main\r*", '*');
	#endif

	for (char ff = 0; ff < 10; ff++){
					makeNock();
					_delay_ms(200);
				}

    while(1)
    {
    	nockMachine_2();
    	sendMachine_3();
    }
}
