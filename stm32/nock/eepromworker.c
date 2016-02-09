#include "eepromworker.h"
//=============================================================================
void eepromInit(){

}
//=============================================================================
void eepromWrite(char * data, char len, unsigned int address){
	char i;
	for (i = 0; i < len; i++) {
		;// eeprom_write_byte(address + i, data[i]);
	}
}
//=============================================================================
void eepromRead(char * buffer, char len, unsigned int address){
	char i;
	for (i = 0; i < len; i++) {
		;// buffer[i] = eeprom_read_byte(address + i);
	}
}
//=============================================================================
void eepromReadToMark(char * buffer, char * len, char marker, unsigned int address){
	char i;
	for (i = 0; i < *len; i++) {
		;// buffer[i] = eeprom_read_byte(address + i);
		if (buffer[i] == marker){
			*len = i + 1;
			return;
		}
	}
}
//=============================================================================
