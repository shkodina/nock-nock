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

  /* �������������� ������� ������: �������� 24000, ������ 500 ��.
   * ������ ��������� ��������� TIM_TimeBaseInitTypeDef
   * �� ����� ������ ��� ������� ��������.
   */
  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);
  /* �������� ����������� ��� TIM_Prescaler + 1, ������� �������� 1 */
  base_timer.TIM_Prescaler = 24000 - 1;
  base_timer.TIM_Period = 500;
  TIM_TimeBaseInit(TIM6, &base_timer);

  /* ��������� ���������� �� ���������� (� ������ ������ -
   * �� ������������) �������� ������� TIM6.
   */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  /* �������� ������ */
  TIM_Cmd(TIM6, ENABLE);

  /* ��������� ��������� ���������� �� ������������ ��������
   * ������� TIM6. ��� ����������, ��� ��� �� ����������
   * �������� � �� ����������� ���.
   */
  NVIC_EnableIRQ(TIM6_DAC_IRQn);
}
//=============================================================================
void TIM6_DAC_IRQHandler()
{
  // ��� ��� ���� ���������� ���������� � ��� ���, ����� ���������,
  // ��������� �� ���������� �� ������������ �������� ������� TIM6.

  if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  {
    // ������� ��� ��������������� ����������
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

    // ������ ������
    inIRQTimer();
  }
}
//=============================================================================
