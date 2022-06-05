#include "scheduler_hal.h"
#include "main.h"

extern SCH_TIM_TypeDef schTIMER;

// HAL state
static struct {
	int irqlevel;
  uint32_t ticks;
} sHAL;

// -----------------------------------------------------------------------------
// IRQ
void hal_disableIRQs() 
{
	__disable_irq();
	sHAL.irqlevel++;
}

void hal_enableIRQs() 
{
	if(--sHAL.irqlevel == 0) 
	{
		__enable_irq();
	}
}

// -----------------------------------------------------------------------------
// TIME
static Std_ReturnType hal_time_init() 
{
	// already done by cubemx
	return STD_OK;
}

uint32_t hal_ticks() 
{
	hal_disableIRQs();
	uint32_t t = sHAL.ticks;
	uint16_t cnt = __HAL_TIM_GET_COUNTER(&schTIMER);
	if(__HAL_TIM_GET_FLAG(&schTIMER, TIM_FLAG_CC1) != RESET)
	{
		if(__HAL_TIM_GET_IT_SOURCE(&schTIMER, TIM_IT_CC1) !=RESET)
		{
			cnt = __HAL_TIM_GET_COUNTER(&schTIMER);
			t++;
		}
	}
	hal_enableIRQs();
	return (t<<16)|cnt;
}

// return modified delta ticks from now to specified ticktime (0 for past, FFFF for far future)
static uint16_t deltaticks(uint32_t time) 
{
	uint32_t t = hal_ticks();
	int d = time - t;
	if(d<=0) return 0;    // in the past
	if((d>>16)!=0) return 0xFFFF; // far ahead
	return (uint16_t)d;
}

void hal_waitUntil(uint32_t time) 
{
	while(deltaticks(time) != 0); // busy wait until timestamp is reached
}

// check and rewind for target time
uint8_t hal_checkTimer(uint32_t time) 
{
	uint16_t dt;
	schTIMER.Instance->SR &= ~TIM_SR_CC1IF; // clear any pending interrupts
	if((dt = deltaticks(time)) < 5) 
	{
		// event is now (a few ticks ahead)
		schTIMER.Instance->DIER &= ~TIM_DIER_CC1IE; // disable IE
		return 1;
	} 
	else 
	{ 
		// rewind timer (fully or to exact time))
		schTIMER.Instance->CCR1 = schTIMER.Instance->CNT + dt;   // set comparator
		schTIMER.Instance->DIER |= TIM_DIER_CC1IE;  // enable IE
		schTIMER.Instance->CCER |= TIM_CCER_CC1E;   // enable capture/compare uint 2
		return 0;
	}
}

// Timer Callback
void HAL_TIM_SCHCallback(TIM_HandleTypeDef *htim) 
{
	if(htim->Instance == schTIMER.Instance)
	{
		sHAL.ticks++;
	}
}

void hal_sleep() 
{
	// low power sleep mode
#ifndef CFG_no_low_power_sleep_mode
	// PWR->CR |= PWR_CR_LPSDSR;
#endif
	// suspend execution until IRQ, regardless of the CPSR I-bit
	__WFI();
}

