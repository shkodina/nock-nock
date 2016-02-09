#include "common_makros.h"
#include "uart_logger.h"
#include "buttons.h"
#include "softtimer.h"
#include "eepromworker.h"


int main(void)
{
	eepromInit();
	buttonsInit();
	ledInit();
	timerInit();

	loggerInit();
	loggerWrite((LogMesT)"Starting program *");

    while(1)
    {
    }
}
