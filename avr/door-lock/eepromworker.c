#include "eepromworker.h"

#include <avr/eeprom.h>

//=============================================================================
void eepromInit(){

}
//=============================================================================
void eepromWrite(char * data, char len, unsigned int address){
	eeprom_write_block(data, address, len);
}
//=============================================================================
void eepromRead(char * buffer, char len, unsigned int address){

	eeprom_read_block(buffer, address, len);
}
//=============================================================================
void eepromReadToMark(char * buffer, char * len, char marker, unsigned int address){
	char i;
	for (i = 0; i < *len; i++) {
		eeprom_read_block(buffer + i, address + i, len);
		if (buffer[i] == marker){
			*len = i + 1;
			return;
		}
	}
}
//=============================================================================
