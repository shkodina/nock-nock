#include "buttons.h"
#include "common_makros.h"

#define BUTTONPORT GPIOA
#define BUTOONNEWCODEPIN 0

#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>


//=============================================================================
inline void gpioInit(){
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); //TODO

    GPIO_InitStructure.GPIO_Pin = (1 << BUTOONNEWCODEPIN);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;

    GPIO_Init( BUTTONPORT , &GPIO_InitStructure);
}
//=============================================================================
void buttonsInit(){
	gpioInit();
}
//=============================================================================
char buttonIsPressed(char button_name){
	return ((BUTTONPORT->IDR & (1 << BUTOONNEWCODEPIN)) > 0) ? TRUE : FALSE;
}
//=============================================================================
