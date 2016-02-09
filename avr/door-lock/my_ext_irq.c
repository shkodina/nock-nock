#include "nock_machine.h"
#include "eepromworker.h"
#include "common_makros.h"
#include "my_ext_irq.h"



#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_VREF_TYPE 0x00

//#define LOGGDEBUG
#ifdef LOGGDEBUG
#include "uart_logger.h"
#endif

volatile unsigned char sence = 20;



#define SENCEMAXVALUE 200
#define SENCEMINVALUE 2
#define SENCESTEP 2

void setNextSence(){
	if (sence >= SENCEMAXVALUE)
		sence = SENCEMINVALUE;
	
	sence += SENCESTEP;		

	eepromWrite(&sence, 1, EEPROM_NOCK_SENCE_ADDRESS);

	#ifdef LOGGDEBUG
	loggerWriteByteInBit(sence);
	#endif
}

void setFixedSence(unsigned char value){
	sence = value;
	eepromWrite(&sence, 1, EEPROM_NOCK_SENCE_ADDRESS);

}

void initMyExtIRQ(){

	eepromRead(&sence, 1, EEPROM_NOCK_SENCE_ADDRESS);
	if (sence < SENCEMINVALUE || sence >= SENCEMAXVALUE)
		sence = (SENCEMAXVALUE - SENCEMINVALUE) / 2;

	#ifdef LOGGDEBUG
	loggerWriteByteInBit(sence);
	sence = 60;
	#endif




	ADCSRA =(1<<ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADFR)|(3<<ADPS0);
	// ���� ��� ��� � ���:
	// ADEN = 1 - ��������� ���
	// ADIE = 1��������� ����������. 
	// ADSC = 1 ��������� �������������� (������, ������ ���������)
	// ADFR = ADATE = 1 ����������� ���������������� ��������������, ���� �� ������.
	// ADPS2..0 = 3 �������� ������� �� 8 - ��� � ���� ���������� ����������� �������. 
 
	ADMUX = 0b01000000;
	//� ��� �������� ������ ����� ����� ������
	//REFS -- 0b[01]000101 ������ ��� ���� - ���������� � ����� AVCC
	//ADLAR --0b01[0]00101��������� ��� ������������ �� ������� ����
	//MUX -- 0b010[00000]</B>������ �� ���� ���� � �������� ������ ���.
	sei();

}

ISR (ADC_vect)
{
	unsigned int adc_data;
// Read the AD conversion result
	adc_data=ADCW;

	if (adc_data > sence){
		//loggerWrite("x", 1);
		changeFlag(FLAG_NOCK, TRUE);
	}
}
