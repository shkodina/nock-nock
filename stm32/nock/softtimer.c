#include "softtimer.h"
#include "common_makros.h"

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include <misc.h>
//=============================================================================
enum {	TIMER_STATE_IN_RUN = 1,
		TIMER_STATE_ELAPSED = 2,
		TIMER_STATE_NEED_RESTART = 4
	};
//=============================================================================
typedef struct {
	unsigned int counter;
	unsigned int threshold;
	char state;
} SoftTimer;
//=============================================================================
//volatile
SoftTimer g_timer[TIMER_TOTAL_COUNT];
//=============================================================================
void timerSet(char timer_name, unsigned int start_value, unsigned int stop_value){
	g_timer[timer_name].counter = start_value;
	g_timer[timer_name].threshold = stop_value;
	g_timer[timer_name].state = TIMER_STATE_IN_RUN;
}
//=============================================================================
char timerIsElapsed(char timer_name){
	if ((g_timer[timer_name].state & TIMER_STATE_ELAPSED) > 0){
		g_timer[timer_name].state &= ~TIMER_STATE_ELAPSED;
		return TRUE;
	}
	return FALSE;
}
//=============================================================================
unsigned int timerGetCurrent(char timer_name){
	return g_timer[timer_name].counter;
}
//=============================================================================
void timerInit(){
	char i;
	for (i = 0; i < TIMER_TOTAL_COUNT; i++){
		g_timer[i].counter = 0;
		g_timer[i].threshold = 0;
		g_timer[i].state = 0;
	}

	initHardWareTimer();
}
//=============================================================================
inline inIRQTimer(){
	char i;
	for (i = 0; i < TIMER_TOTAL_COUNT; i++){
		if ( (g_timer[i].state & TIMER_STATE_IN_RUN) > 0){
			if (g_timer[i].counter++ >= g_timer[i].threshold){
				g_timer[i].state &=	~TIMER_STATE_IN_RUN;
				g_timer[i].state |= TIMER_STATE_ELAPSED;
			}
		}
	}
}
//=============================================================================
// HARDWARE
//=============================================================================
void initHardWareTimer()
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Инициализируем базовый таймер: делитель 24000, период 500 мс.
   * Другие параметры структуры TIM_TimeBaseInitTypeDef
   * не имеют смысла для базовых таймеров.
   */
  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);
  /* Делитель учитывается как TIM_Prescaler + 1, поэтому отнимаем 1 */
  base_timer.TIM_Prescaler = 24000 - 1;
  base_timer.TIM_Period = 500;
  TIM_TimeBaseInit(TIM6, &base_timer);

  /* Разрешаем прерывание по обновлению (в данном случае -
   * по переполнению) счётчика таймера TIM6.
   */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  /* Включаем таймер */
  TIM_Cmd(TIM6, ENABLE);

  /* Разрешаем обработку прерывания по переполнению счётчика
   * таймера TIM6. Так получилось, что это же прерывание
   * отвечает и за опустошение ЦАП.
   */
  NVIC_EnableIRQ(TIM6_DAC_IRQn);
}
//=============================================================================
void TIM6_DAC_IRQHandler()
{
  // Так как этот обработчик вызывается и для ЦАП, нужно проверять,
  // произошло ли прерывание по переполнению счётчика таймера TIM6.

  if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  {
    // Очищаем бит обрабатываемого прерывания
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

    // делаем работу
    inIRQTimer();
  }
}
//=============================================================================
