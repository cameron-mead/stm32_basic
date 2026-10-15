#ifndef __NUCLEO476_GPIO_H
#define __NUCLEO476_GPIO_H

#include "stm32l476xx.h"

//
// pin declarations
//

//  port a ( GPIOA ) = 0x00 | pin
#define POTENTIOMETER       ( 0x00 | 0  )   // gpio PA0     // Arduino A0
                         // ( 0x00 | 1  )   // gpio PA1     // Arduino A1
#define USART_TX            ( 0x00 | 2  )   // gpio PA2     // Arduino D1       // RESERVED USART_2 TX
#define USART_RX            ( 0x00 | 3  )   // gpio PA3     // Arduino D0       // RESERVED USART_2 RX
                         // ( 0x00 | 4  )   // gpio PA4     // Arduino A2
#define BOARD_LED 	        ( 0x00 | 5  )   // gpio PA5     // Arduino D13      // green board led
#define GREEN_LED 	        ( 0x00 | 6  )   // gpio PA6     // Arduino D12
#define RED_LED   	        ( 0x00 | 7  )   // gpio PA7     // Arduino D11
#define RANGER_ECHO         ( 0x00 | 8  )   // gpio PA8     // Arduino D7
#define RANGER_TRIGGER      ( 0x00 | 9  )   // gpio PA9     // Arduino D8
#define RANGER_BUTTON       ( 0x00 | 10 )   // gpio PA10    // Arduino D2
                         // ( 0x00 | 11 )   // gpio PA11                        // USB D-
                         // ( 0x00 | 12 )   // gpio PA12                        // USB D+
// RESERVED              // ( 0x00 | 13 )   // gpio PA13                        // RESERVED SWDIO
// RESERVED              // ( 0x00 | 14 )   // gpio PA14                        // RESERVED SWCLK
                         // ( 0x00 | 15 )   // gpio PA15 

//  port b ( GPIOB ) = 0x10 | pin
                         // ( 0x10 | 0  )   // gpio PB0     // Arduino A3
                         // ( 0x10 | 1  )   // gpio PB1 
                         // ( 0x10 | 2  )   // gpio PB2
						 // ( 0x10 | 3  )   // gpio PB3     // Arduino D3
                         // ( 0x10 | 4  )   // gpio PB4     // Arduino D5
                         // ( 0x10 | 5  )   // gpio PB5     // Arduino D4
                         // ( 0x10 | 6  )   // gpio PB6     // Arduino D10
                         // ( 0x10 | 7  )   // gpio PB7
                         // ( 0x10 | 8  )   // gpio PB8     // Arduino D15      // I2C SCL
                         // ( 0x10 | 9  )   // gpio PB9     // Arduino D14      // I2C SDA
                         // ( 0x10 | 10 )   // gpio PB10    // Arduino D6
                         // ( 0x10 | 11 )   // gpio PB11
                         // ( 0x10 | 12 )   // gpio PB12
                         // ( 0x10 | 13 )   // gpio PB13
                         // ( 0x10 | 14 )   // gpio PB14
                         // ( 0x10 | 15 )   // gpio PB15

//  port c ( GPIO C ) = 0x20 | pin
                         // ( 0x20 | 0  )   // gpio PC0     // Arduino A5
                         // ( 0x20 | 1  )   // gpio PC1     // Arduino A4
                         // ( 0x20 | 2  )   // gpio PC2
                         // ( 0x20 | 3  )   // gpio PC3
                         // ( 0x20 | 4  )   // gpio PC4
                         // ( 0x20 | 5  )   // gpio PC5
                         // ( 0x20 | 6  )   // gpio PC6
                         // ( 0x20 | 7  )   // gpio PC7     // Arduino D9
                         // ( 0x20 | 8  )   // gpio PC8 
                         // ( 0x20 | 9  )   // gpio PC9 
                         // ( 0x20 | 10 )   // gpio PC10
                         // ( 0x20 | 11 )   // gpio PC11
                         // ( 0x20 | 12 )   // gpio PC12
#define USER_BUTTON         ( 0x20 | 13 )   // gpio PC13                        // user button ( B1 )
// RESERVED              // ( 0x20 | 14 )   // gpio PC14                        // RESERVED 32.768 kHz crystal in
// RESERVED              // ( 0x20 | 15 )   // gpio PC15                        // RESERVED 32.768 kHz crystal out



//
// configuration constants
//

// pin modes
#define INPUT 		        0
#define OUTPUT 		        1
#define ALTERNATE	        2
#define ANALOG		        3

// pull resistors
#define NO_PULL 	        0
#define PULL_UP 	        1
#define PULL_DOWN 	        2

// speed
#define LOW_SPEED			0
#define MEDIUM_SPEED		1
#define FAST_SPEED		    2
#define HIGH_SPEED		    3

// output types
#define PUSH_PULL	        0
#define OPEN_DRAIN	        1

// interrupt edges
#define RISING_EDGE	        1
#define FALLING_EDGE        2
#define BOTH_EDGE	        3

// basic states
#define ON			        1
#define HIGH                1
#define OFF 		        0
#define LOW                 0


typedef void ( * callback_t ) ( void ); // callback function declaration

typedef struct {                        // interrupt pin and callback struct
    uint32_t pin;
    callback_t callback;
} interrupt_t;

// ON, OFF
void 	 gpio_output            ( uint8_t pin, uint8_t state );
void 	 gpio_toggle            ( uint8_t pin );
uint8_t  gpio_input 	        ( uint8_t pin );
uint32_t gpio_analog_input      ( uint8_t pin );

uint8_t mask_to_pin             ( uint16_t pin_mask );
void    HAL_GPIO_EXTI_Callback  ( uint16_t pin );

// INPUT, OUTPUT, ALTERNATE, ANALOG
void configure_gpio_mode 		( uint8_t pin, uint8_t mode );

// NO_PULL, PULL_UP, PULL_DOWN
void configure_gpio_pull 		( uint8_t pin, uint8_t pull );

// LOW_SPEED, MEDIUM_SPEED, FAST_SPEED, HIGH_SPEED
void configure_gpio_speed		( uint8_t pin, uint8_t speed);

// PUSH_PULL, OPEN_DRAIN
void configure_gpio_output_type ( uint8_t pin, uint8_t type );

// RISING_EDGE, FALLING_EDGE, BOTH_EDGE
void configure_gpio_as_interrupt( uint8_t pin, uint8_t edge, callback_t callback );

uint32_t pin_to_channel         ( uint8_t pin );

#endif /* __NUCLEO476_GPIO_H */
