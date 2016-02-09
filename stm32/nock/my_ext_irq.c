#include "nock_machine.h"
#include "common_makros.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"

#define KEY_BUTTON_IRQn             EXTI9_5_IRQn
#define KEY_BUTTON_NOCK_EXTI_LINE        EXTI_Line8
#define KEY_BUTTON_NEW_NOCK_EXTI_LINE        EXTI_Line9

void initMyExtIRQ(){
	  GPIO_InitTypeDef GPIO_InitStructure;
	  EXTI_InitTypeDef EXTI_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;

	  /* Configure the Key button : GPIOB_9(other board may be GPIOG_8) */

	  /* Enable Button GPIO clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	  /* Configure Button pin as input floating */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  GPIO_Init(GPIOB, &GPIO_InitStructure);

	  /* Connect Button EXTI Line to Button GPIO Pin */
	  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
	  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

	  /* Configure Button EXTI line */
	  EXTI_InitStructure.EXTI_Line = EXTI_Line8 | EXTI_Line9;
	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	  EXTI_Init(&EXTI_InitStructure);

	  /* Enable and set Button EXTI Interrupt to the lowest priority */
	  NVIC_InitStructure.NVIC_IRQChannel = KEY_BUTTON_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	  NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(KEY_BUTTON_NOCK_EXTI_LINE) != RESET)
  {
    // Clear KEY_BUTTON_EXTI_LINE pending bit
    EXTI_ClearITPendingBit(KEY_BUTTON_NOCK_EXTI_LINE);

   	changeFlag(FLAG_NOCK, TRUE);
  }

  if(EXTI_GetITStatus(KEY_BUTTON_NEW_NOCK_EXTI_LINE) != RESET)
  {
    // Clear KEY_BUTTON_EXTI_LINE pending bit
    EXTI_ClearITPendingBit(KEY_BUTTON_NEW_NOCK_EXTI_LINE);

   	changeFlag(FLAG_NEW_NOCK, TRUE);
  }

}
