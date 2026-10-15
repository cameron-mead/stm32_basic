#ifndef __NUCLEO476_UART_H
#define __NUCLEO476_UART_H

#include "stm32l476xx.h"

#define TX_PIN 2
#define RX_PIN 3

#define BACKSPACE_CODE 0x7F
#define HIDE_CURSOR "\x1B[?25l"
#define SHOW_CURSOR "\033[?25h"

#define UART_BUFFER_SIZE (32)
#define COMMAND_BUFFER_SIZE 20

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes);
void USART_Delay(uint32_t us);
void USART_IRQHandler(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t * pRx_counter);
uint8_t USART_Read_nonblocking (USART_TypeDef * USARTx);

/*
 * return uppercase version of character, else character
 * */
uint8_t to_uppercase ( uint8_t character );

/*
 * get character from USART2
 * */
uint8_t get_character ( void );

/*
 * puts character to USART2
 * */
void put_character ( uint8_t character );

/*
 *  put character without processing to USART2
 * */
void put_character_literal ( uint8_t character );

/*
 * puts number to USART2
 * */
void put_number ( uint32_t number );

/*
 * puts string to USART2
 * */
void put_string ( char *string );

/*
 * hide cursor
 * */
void hide_cursor ( );

/*
 * show cursor
 * */
void show_cursor ( );

/*
 * handle user input
 * send full command to application.c
 * */
void handle_input ( );

#endif /* __NUCLEO476_UART_H */
