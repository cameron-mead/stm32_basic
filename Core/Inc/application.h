// SWEN 340 Demo code header file
// Larry Kiser copyright 2021

#include <string.h>

#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROJECT_SETUP_H
#define __PROJECT_SETUP_H

enum application_mode {
	NONE	= 0,
	LED 	= 1,
	RANGER	= 2
};

/*
 * print current mode and prompt string
 * */
void prompt ( );

/*
 * handle command
 * called from UART.c
 * */
void handle_command ( char *command );

/*
 * main loop
 * */
void main_loop ( void ) ;

#endif /* __PROJECT_SETUP_H */
