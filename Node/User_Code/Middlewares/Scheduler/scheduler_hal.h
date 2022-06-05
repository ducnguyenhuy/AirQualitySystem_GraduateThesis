#ifndef SCHEDULER_HAL_H
#define SCHEDULER_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "./cfg/scheduler_cfg.h"

void hal_disableIRQs();

void hal_enableIRQs();

uint32_t hal_ticks();
	
void hal_waitUntil(uint32_t time);

uint8_t hal_checkTimer(uint32_t time);

void hal_sleep();

void HAL_TIM_SCHCallback(TIM_HandleTypeDef *htim);
#ifdef __cplusplus
}
#endif
#endif /* SCHEDULER_HAL_H */

