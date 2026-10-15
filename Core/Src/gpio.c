
#include "gpio.h"
#include "systick.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "main.h"

static uint8_t      number_interrupts = 0;  // current total number of interrupts
static interrupt_t  interrupts [16] = {0};  // current interrupts pins and callbacks
                                            // only 16 possible at a given time

//
//
//	pin manipulation functions
//
//

/*
 * convert encoded pin number to gpio port
 * */
GPIO_TypeDef *to_port ( uint8_t pin ) {
    switch ( pin >> 4 ){
        case 0:
            return GPIOA;
        case 1:
            return GPIOB;
        case 2:
            return GPIOC;
        default:
            return NULL;
    }
}

/*
 * convert encoded pin number to plain pin number
 * */
uint8_t to_pin_number ( uint8_t pin ) {
    return pin & 0x0F;
}

/*
 * turn on or off gpio
 * */
void gpio_output ( uint8_t pin, uint8_t state ){
    GPIO_TypeDef *GPIO = to_port        ( pin );
    uint8_t pin_number = to_pin_number  ( pin );

    configure_gpio_mode ( pin, OUTPUT );
	if ( state ){
		GPIO->ODR |= (uint32_t)1 << pin_number;
	} else {
		GPIO->ODR &= ~( (uint32_t)1 << pin_number );
	}
}

/*
 * toggle gpio
 * */
void gpio_toggle ( uint8_t pin ) {
    GPIO_TypeDef *GPIO = to_port        ( pin );
    uint8_t pin_number = to_pin_number  ( pin );

    configure_gpio_mode ( pin, OUTPUT );
	GPIO->ODR ^= (uint32_t)1 << pin_number ;
}

/*
 * return state of gpio pin
 * */
uint8_t gpio_input ( uint8_t pin ) {
    GPIO_TypeDef *GPIO = to_port        ( pin );
    uint8_t pin_number = to_pin_number  ( pin );

    configure_gpio_mode ( pin, INPUT );
	return ( GPIO->IDR >> pin_number ) & 1 ; // shift mask to position 0 and isolate bit
}

/*
 * return current state of analog pin
 * */
uint32_t gpio_analog_input ( uint8_t pin ){
    
    configure_gpio_mode( pin, ANALOG );
    uint32_t channel = pin_to_channel ( pin );

    ADC_ChannelConfTypeDef ch = {0};        // configure for channel
    ch.Channel      = channel;
    ch.Rank         = ADC_REGULAR_RANK_1;
    ch.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    ch.SingleDiff   = ADC_SINGLE_ENDED;
    ch.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc1, &ch);

    HAL_ADC_Start(&hadc1);                  // perform conversion
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);                   // stop adc
    return raw;                             // return value
}

//
//
//	interrupt functions
//
//

/*
 * helper for HAL_GPIO_EXTI_Callback
 * returns pin number from pin mask
 * */
uint8_t mask_to_pin ( uint16_t pin_mask ){
	uint8_t pin_number = 0;
	while ( pin_mask >>= 1 ){	// shift right until 0
		pin_number ++;					// increment pin number
	}
	return pin_number;
}

/*
 * gpio interrupt handler
 * called automatically by system
 * */
void HAL_GPIO_EXTI_Callback ( uint16_t pin_mask ){

    uint8_t pin = mask_to_pin ( pin_mask );                                 // convert pin mask to pin number

    for ( uint8_t index = 0 ; index < number_interrupts ; index ++ ){       // for each registered interrupt
        interrupt_t *interrupt = &interrupts [ index ];                     // fetch struct
        uint8_t pin_number = to_pin_number ( interrupt -> pin );
        if ( pin_number == pin && interrupt -> callback ) {           // if pin matches and callback not null
            interrupt -> callback ();                                       // call callback
            return;
        }
    }

}
        
void EXTI0_IRQHandler( ) { 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);                                   // call interrupt on pin 0
}
void EXTI9_5_IRQHandler( ) {
    for (uint32_t i = 5; i <= 9; i++) HAL_GPIO_EXTI_IRQHandler(1 << i);     // call interrupt on any pins
}                                                                           // 5 - 9
void EXTI15_10_IRQHandler( ) {
    for (uint32_t i = 10; i <= 15; i++) HAL_GPIO_EXTI_IRQHandler(1 << i);   // call interrupt on any pins
}                                                                           // 10 - 15

//
//
//	pin configuration functions
//
//

/*
 * configure mode of gpio pin
 * INPUT, OUTPUT, ALTERNATE, or ANALOG
 * */
void configure_gpio_mode ( uint8_t pin, uint8_t mode ) {
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin );

	GPIO->MODER &= ~((uint32_t)3	<< (2 * pin_number));  		    // clear bits
	GPIO->MODER |=   (uint32_t)mode << (2 * pin_number);   		    // set new value
    
    if ( mode == ANALOG ){
        configure_gpio_pull( pin, NO_PULL );                        // clear pull resistor
        GPIO->ASCR |= (1U << pin_number);                           // close analog switch
    }
}

/*
 * configure pull of gpio pin
 * NO_PULL, PULL_UP, or PULL_DOWN
 * */
void configure_gpio_pull ( uint8_t pin, uint8_t pull ) {
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin );

	GPIO->PUPDR	&= ~( (uint32_t)3 		<< ( 2 * pin_number ) ) ;	// clear bits
	GPIO->PUPDR	|= 	  (uint32_t)pull	<< ( 2 * pin_number ) ;	    // set new value
}

/*
 * configure speed of gpio pin
 * LOW_SPEED, MEDIUM_SPEED, FAST_SPEED, or HIGH_SPEED
 * */
void configure_gpio_speed ( uint8_t pin, uint8_t speed ) {
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin );

	GPIO->OSPEEDR &= ~( (uint32_t)3 	<< ( 2 * pin_number ) ) ;	// clear bits
	GPIO->OSPEEDR |=    (uint32_t)speed << ( 2 * pin_number ) ;	    // set new value
}

/*
 * configure output type of gpio pin
 * PUSH_PULL or OPEN_DRAIN
 * */
void configure_gpio_output_type ( uint8_t pin, uint8_t type ){
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin );

	GPIO->OTYPER &= ~( (uint32_t)1    << pin_number ) ;             // clear bits
	GPIO->OTYPER |=	   (uint32_t)type << pin_number  ;              // set new value
}

/*
 * configure gpio pin as interrupt
 * on edge RISING_EDGE, FALLING_EDGE, or BOTH_EDGE
 * on trigger calls callback function
 * */
void configure_gpio_as_interrupt ( uint8_t pin, uint8_t edge, callback_t callback ){
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin );


    interrupts [ number_interrupts ] = ( interrupt_t ) { pin, callback };   // add pin and callback to struct
    number_interrupts ++;                                                   // increment size
    
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;                                   // enable SYSCFG clock
    configure_gpio_mode ( pin, INPUT );                                     // configure as input

    uint32_t port = ((uint32_t)GPIO - (uint32_t)GPIOA) / 0x400;
    SYSCFG->EXTICR[pin / 4] &= ~(0xFU << ((pin_number % 4) * 4));           // determine which port to use
    SYSCFG->EXTICR[pin / 4] |=  (port << ((pin_number % 4) * 4));

    if (edge & RISING_EDGE)  EXTI->RTSR1 |= (1 << pin_number);              // set rising edge
    if (edge & FALLING_EDGE) EXTI->FTSR1 |= (1 << pin_number);              // set falling edge
                                                                            // sets both if both
  
    EXTI->IMR1 |= (1 << pin_number);                                        // set interrupt bit in mask

    IRQn_Type irq;                                                          // determine which IRQ handler
    if      (pin_number <= 4) irq = (IRQn_Type)(EXTI0_IRQn + pin_number);   // is called based on which pin
    else if (pin_number <= 9) irq = EXTI9_5_IRQn;                          
    else               irq = EXTI15_10_IRQn;
    NVIC_EnableIRQ(irq);

}

//
//
//	setup functions
//
//


/*
 * return analog channel from port and pin numbers
 * */
uint32_t pin_to_channel ( uint8_t pin ) {
    GPIO_TypeDef *GPIO = to_port       ( pin );
    uint8_t pin_number = to_pin_number ( pin ); 

	if ( GPIO == GPIOA) {
		switch ( pin_number ){
			case 0:
				return ADC_CHANNEL_5;
			case 1:
				return ADC_CHANNEL_6;
			case 2:
				return ADC_CHANNEL_7;
			case 3:
				return ADC_CHANNEL_8;
			case 4:
				return ADC_CHANNEL_9;
			case 5:
				return ADC_CHANNEL_10;
			case 6:
				return ADC_CHANNEL_11;
			case 7:
				return ADC_CHANNEL_12;
		}
	} else if ( GPIO == GPIOB ) {
		switch ( pin_number ) {
			case 0:
				return ADC_CHANNEL_15;
			case 1:
				return ADC_CHANNEL_16;
		}
	} else if ( GPIO == GPIOC ){
		switch ( pin_number ) {
			case 0:
				return ADC_CHANNEL_1;
			case 1:
				return ADC_CHANNEL_2;
			case 2:
				return ADC_CHANNEL_3;
			case 3:
				return ADC_CHANNEL_4;
			case 4:
				return ADC_CHANNEL_13;
			case 5:
				return ADC_CHANNEL_14;
		}
	}

	return 0;

}
