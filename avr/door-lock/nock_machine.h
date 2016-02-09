#ifndef MY_NOCK_MACHINE_H
#define MY_NOCK_MACHINE_H
//=============================================================================
enum {	FLAG_TOTAL_COUNT = 5,
		FLAG_NOCK = 0,
		FLAG_NOCK_CORRECT = 1,
		FLAG_NEW_NOCK = 2,
		FLAG_ERROR = 3};
//=============================================================================
enum {	NOCK_MAX_COUNT = 30,
		NOCK_NO_NOCK = 0,
		NOCK_EEPROM_START_ADDRESS = 1,
		NOCK_DELTA = 400};
//=============================================================================
typedef struct {
	int nock[NOCK_MAX_COUNT];
	char count;
} Nock;
//=============================================================================
enum {	NOCK_G_NOCK_TOTAL_COUNT = 3,
		NOCK_PATTERN = 0,
		NOCK_CURRENT = 1,
		NOCK_NEW = 2};
//=============================================================================
void initNockMachine();
void userNewNockCommandMachine_1();
void nockMachine_2();

void changeFlag(unsigned char name, char state);
char getFlag(unsigned char name);
Nock * getNock(char name);

//=============================================================================
#endif
