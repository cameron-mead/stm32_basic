
#include "stm32l4xx.h"
#include "systick.h"
#include "gpio.h"
#include "UART.h"
#include "application.h"

static volatile uint32_t systick_counter = 0;

/*
 * toggle interrupt enable bit in systick CSR
 * */
void toggle_systick_interrupt () {
    SysTick -> CTRL ^= (1 << 1);
}

/*
 * access reload value of systick timer
 * */
uint32_t get_systick_reload ( ) {
    return ( SysTick -> LOAD & 0x00FFFFFF );
}

/*
 * access current value of systick timer
 * */
uint32_t get_systick_current ( ) {
    return ( SysTick -> VAL & 0x00FFFFFF );
}

/*
 * systick interrupt handler
 * called automatically by system once per millisecond
 * */
void SysTick_Handler(){

    systick_counter ++;                     // increment systick_counter for 1s interrupt
    if ( systick_counter >= 1000 ){         // if one second elapsed
        systick_counter = 0;                // reset systick_counter
        handle_second();                    // call one second interrupt
    }

}

/*
 * 1 second interrupt handler
 * called by SysTick Handler once per second
 * */
void handle_second ( ) {

                        
}

