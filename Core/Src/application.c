
#include <string.h>

#include "application.h"
#include "UART.h"
#include "gpio.h"
#include "systick.h"

#define PROMPT "\r >> "

static volatile uint8_t suppress_prompt = 0;

static volatile enum application_mode mode = NONE;

static char *mode_strings[] = {
	"MODE SELECT",
	NULL
};

void prompt ( ) {
	if ( !suppress_prompt ){
		put_character ( '\r' );
		put_string ( mode_strings [ mode ] );
		put_string ( PROMPT );
	}
}

/*
 * handle command
 * called automatically from UART.c
 * */
void handle_command ( char *command ) {

	switch ( mode ) {

    	case NONE:
    	default:

    	    break;

    }
}

/*
 * main loop
 * */
void main_loop () {


    while (1) {                 // main loop

        handle_input();         // handle user input

	}

}
