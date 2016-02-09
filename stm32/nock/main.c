#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "softtimer.h"
#include "eepromworker.h"

void initMyExtIRQ();

int main(void)
{
	initMyExtIRQ();

	eepromInit();
	buttonsInit();
	ledInit();
	timerInit();

	loggerInit();
	loggerWriteToMarker((LogMesT)" \nStarting program \n*", '*');

	initNockMachine_0();


    while(1)
    {

    	userCommandMachine_1();
    	nockMachine_2();
    	radioSendMachine_3();


    }
}
