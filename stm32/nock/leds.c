#include "leds.h"
#include "common_makros.h"

#define LEDPORT		GPIOC
char led_pins[LEDTOTAL] = {	8, 	// LEDGREEN
            		  	  	9 	// LEDBLUE
              	  	  	  };

//volatile

#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
//=============================================================================
void ledInit(){

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE); // TODO

    int ports = 0;
    char i;
    for (i = 0; i < LEDTOTAL; i++)
    	ports |= (1 << led_pins[i]);
    GPIO_InitStructure.GPIO_Pin = ports;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init( LEDPORT , &GPIO_InitStructure);
}
//=============================================================================
void ledOn(char led_name){
	UPBIT(LEDPORT->ODR, led_pins[led_name]);
}
//=============================================================================
void ledOff(char led_name){
	DOWNBIT(LEDPORT->ODR, led_pins[led_name]);
}//=============================================================================
void ledTaggle(char led_name){
	INVBIT(LEDPORT->ODR, led_pins[led_name]);
}//=============================================================================
