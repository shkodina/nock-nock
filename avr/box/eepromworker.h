#ifndef MY_EEPROM_WORKER_H
#define MY_EEPROM_WORKER_H

enum {EEPROM_NOCK_START_ADDRESS = 12};

void eepromInit();
void eepromWrite(char * data, char len, unsigned int address);
void eepromRead(char * buffer, char len, unsigned int address);
void eepromReadToMark(char * buffer, char * len, char marker, unsigned int address);

#endif
