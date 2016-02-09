#ifndef MY_NOCK_MACHINE_H
#define MY_NOCK_MACHINE_H
//=============================================================================
enum {	FLAG_TOTAL_COUNT = 5,
		FLAG_NOCK = 0,
		FLAG_NOCK_CORRECT = 1,
		FLAG_NEW_NOCK = 2};
//=============================================================================
enum {	NOCK_MAX_COUNT = 30,
		NOCK_NO_NOCK = 0,
		NOCK_EEPROM_START_ADDRESS = 1,
		NOCK_DELTA = 5};
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
void initNockMachine_0();
void userCommandMachine_1();
void nockMachine_2();
void radioSendMachine_3();
void changeFlag(char name, char state);
//=============================================================================
#endif
