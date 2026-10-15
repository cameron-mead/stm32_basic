
#include <stdio.h>

#include "UART.h"
#include "application.h"


uint8_t USART_Read_nonblocking (USART_TypeDef * USARTx) {
	if (USARTx->ISR & USART_ISR_RXNE){			// if RX not empty
		return ((uint8_t)(USARTx->RDR & 0xFF));	// return character
	}
	return '\0';								// else return \0
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	int i;
	// TXE is cleared by a write to the USART_DR register.
	// TXE is set by hardware when the content of the TDR 
	// register has been transferred into the shift register.
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));   	// wait until TXE (TX empty) bit is set
		// Writing USART_DR automatically clears the TXE flag 	
		USARTx->TDR = buffer[i] & 0xFF;
		USART_Delay(300);
	}
	while (!(USARTx->ISR & USART_ISR_TC));   		  // wait until TC bit is set
	USARTx->ISR &= ~USART_ISR_TC;
}   
 

void USART_Delay(uint32_t us) {
	uint32_t time = 100*us/7;    
	while(--time);   
}

void USART_IRQHandler(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t * pRx_counter){
	if(USARTx->ISR & USART_ISR_RXNE) {					// Received data                         
		buffer[*pRx_counter] = USARTx->RDR;             // Reading USART_DR automatically clears the RXNE flag 
		(*pRx_counter)++;  
		if((*pRx_counter) >= UART_BUFFER_SIZE )  {
			(*pRx_counter) = 0;
		}   
	} else if(USARTx->ISR & USART_ISR_TXE) {
 		//USARTx->ISR &= ~USART_ISR_TXE;                // clear interrupt 
		//Tx1_Counter++;
	} else if(USARTx->ISR & USART_ISR_ORE) {			// Overrun Error
		while(1);
	} else if(USARTx->ISR & USART_ISR_PE) {				// Parity Error
		while(1);
	} else if(USARTx->ISR & USART_ISR_PE) {				// USART_ISR_FE	
		while(1);
	} else if (USARTx->ISR & USART_ISR_NE){ 			// Noise Error Flag
		while(1);     
	}	
}

//
//
// virtual terminal functions
//
//

/*
 * convert lowercase character to uppercase
 * */
uint8_t to_uppercase ( uint8_t character ){

	if ( character >= 97 && character <= 122 ){
		return ( character - 32 );
	}
	return character;
}

/*
 * get character from USART2
 * */
uint8_t get_character ( ) {
    return USART_Read_nonblocking( USART2 );
}

/*
 * puts character to USART2
 * */
void put_character ( uint8_t character ) {
	if ( character == BACKSPACE_CODE ){		// handle backspace
		put_string( "\b \b" );
		return;
	}
	if ( character == '\r' ){				// handle newline
	    put_character( '\n' );
	}

    USART_Write( USART2, &character, 1 );	// print character normally
}

/*
 *  put character without processing to USART2
 * */
void put_character_literal ( uint8_t character ) {
    USART_Write( USART2, &character, 1 );	// print character
}

/*
 * puts number to USART2
 * */
void put_number ( uint32_t number ) {
    char buffer[20];
    sprintf(buffer, "%lu", number);
    put_string( buffer );
}

/*
 * puts string to USART2
 * */
void put_string ( char *string ) {
    while ( *string != '\0' ){              // print until end of string marker
        put_character ( *string );    		// put character
        string++;                           // increment string pointer
    }
}

/*
 * hide cursor
 * */
void hide_cursor ( ) {
    put_string ( HIDE_CURSOR );
}

/*
 * show cursor
 * */
void show_cursor ( ) {
    put_string ( SHOW_CURSOR );
}

/*
 * handle user input
 * */
void handle_input ( ) {

	static char command_buffer[COMMAND_BUFFER_SIZE] = {'\0'};   // buffer to hold commands
	static uint8_t command_index = 0;                   	    // current index of array
	static uint8_t input_char = 0 ;     	            	    // input character buffer

    input_char = get_character() ;      	            	    // fetch input character

    if ( input_char ){

        input_char = to_uppercase( input_char );
        put_character( input_char );		            	    // echo character back to virtual terminal

        switch( input_char ) {
            
            case '\r' : 									    // handle enter                    

                command_buffer[command_index] = '\0';		    // insert null terminator
                handle_command ( command_buffer );			    // handle command
                command_index = 0;							    // reset index
                prompt ();
                break;

            case BACKSPACE_CODE : 							    // handle backspace
                if ( command_index <= 0 ){					    // if start of line
                    put_character(' ');						    // do nothing
                } else {									    // else
                    command_index --;						    // decrement index
                }
                break;

            default :										    // handle other character
                if ( command_index >= COMMAND_BUFFER_SIZE ){    // handle oversized command
                    put_string("\r error: command too long \r");
                    command_index = 0;
                    prompt ();
                } else {                                        // properly formatted command
                    command_buffer[command_index] = input_char;
                    command_index ++;
                }
        }
    }

}

