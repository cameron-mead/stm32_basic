#ifndef __NUCLEO476_SYSTICK_H
#define __NUCLEO476_SYSTICK_H

#include "stm32l476xx.h"

void init_systick ();
uint32_t get_systick_reload ();
uint32_t get_systick_current ();
void toggle_systick_interrupt ();
void SysTick_Handler();
void handle_second();

#endif /* __NUCLEO476_SYSTICK_H */
