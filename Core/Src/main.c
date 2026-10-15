
#include "application.h"
#include "UART.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "systick.h"
#include "gpio.h"

#define SYSTICK_RELOAD 80000 // 1ms timer ( time(seconds) = SYSTICK_RELOAD / 80,000,000 )

ADC_HandleTypeDef hadc1; // define adc handle

void adc_init ( ) {

    __HAL_RCC_ADC_CLK_ENABLE();                                     // enable adc clock

    hadc1.Instance                  = ADC1;
    hadc1.Init.ClockPrescaler       = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution           = ADC_RESOLUTION_12B;         
    hadc1.Init.DataAlign            = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode         = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection         = ADC_EOC_SINGLE_CONV;
    hadc1.Init.ContinuousConvMode   = DISABLE;
    hadc1.Init.NbrOfConversion      = 1;
    hadc1.Init.ExternalTrigConv     = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    HAL_ADC_Init(&hadc1);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);          // calibrate adc
                                                                    // others enabled elsewhere
}

void gpio_init ( ) {

	// enable gpio clocks for all ports
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN ;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN ;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN ;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN ;

	(void) RCC->AHB2ENR; // perform read to allow time to initialize
}

void usart2_init( ) {

    // enable clocks
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; 
    (void)RCC->APB1ENR1;   
    RCC->CCIPR &= ~RCC_CCIPR_USART2SEL;
	RCC->CCIPR |=  RCC_CCIPR_USART2SEL_0;

    configure_gpio_mode         ( USART_TX, ALTERNATE   );
    configure_gpio_speed        ( USART_TX, HIGH_SPEED  );
    configure_gpio_pull         ( USART_TX, PULL_UP     );
    configure_gpio_output_type  ( USART_TX, PUSH_PULL   );

    configure_gpio_mode         ( USART_RX, ALTERNATE   );
    configure_gpio_speed        ( USART_RX, HIGH_SPEED  );
    configure_gpio_pull         ( USART_RX, PULL_UP     );
    configure_gpio_output_type  ( USART_RX, PUSH_PULL   );

    GPIOA->AFR[0] &= ~((0xFU << 8) | (0xFU << 12)); // clear both fields
    GPIOA->AFR[0] |=  ((0x7U << 8) | (0x7U << 12)); // set alternate function
    
	USART2->CR1 &= ~USART_CR1_UE;                   // disable USART
	USART2->CR1 &= ~USART_CR1_M;                    // 8 bit word length
	USART2->CR1 &= ~USART_CR1_OVER8;                // oversampling by 16
	USART2->CR2 &= ~USART_CR2_STOP;                 // 1 stop bit

	USART2->BRR  = 0x208D;                          // set baud rate 9600
                                        
	USART2->CR1  |= (USART_CR1_RE | USART_CR1_TE);  // tx/rx enable

	USART2->ICR |= USART_ICR_TCCF;

	USART2->CR1  |= USART_CR1_UE;                   // enable USART
	while ( (USART2->ISR & USART_ISR_TEACK) == 0);  // Verify that the USART is ready for reception
	while ( (USART2->ISR & USART_ISR_REACK) == 0);  // Verify that the USART is ready for transmission
}

/*
 * initializes and enables systick with a 1ms timer
 * */
void init_systick () {
	SysTick -> CTRL &= 0xFFFFFFFF << 3;     // clear bottom 3 bits of CSR
	SysTick -> LOAD  = SYSTICK_RELOAD - 1;  // load SYSTICK_RELOAD - 1 as reload value
	SysTick -> CTRL |= (1 << 2);            // set clock source to internal
    SysTick -> CTRL |= (1 << 1);            // set interrupt bit
	SysTick -> CTRL |= (1 << 0);            // set enable bit
}

/*
 * initialize system clock
 * PLL source from MSI to HSI
 * PLL as SYSCLK source
 * */
void System_Clock_Init(void){
	
	uint32_t HSITrim;

	// To correctly read data from FLASH memory, the number of wait states (LATENCY)
  // must be correctly programmed according to the frequency of the CPU clock
  // (HCLK) and the supply voltage of the device.		
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |=  FLASH_ACR_LATENCY_2WS;
		
	// Enable the Internal High Speed oscillator (HSI
	RCC->CR |= RCC_CR_HSION;
	while((RCC->CR & RCC_CR_HSIRDY) == 0);
	// Adjusts the Internal High Speed oscillator (HSI) calibration value
	// RC oscillator frequencies are factory calibrated by ST for 1 % accuracy at 25oC
	// After reset, the factory calibration value is loaded in HSICAL[7:0] of RCC_ICSCR	
	HSITrim = 16; // user-programmable trimming value that is added to HSICAL[7:0] in ICSCR.
	RCC->ICSCR &= ~RCC_ICSCR_HSITRIM;
	RCC->ICSCR |= HSITrim << 24;
	
	RCC->CR    &= ~RCC_CR_PLLON; 
	while((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY);
	
	// Select clock source to PLL
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI; // 00 = No clock, 01 = MSI, 10 = HSI, 11 = HSE
	
	// Make PLL as 80 MHz
	// f(VCO clock) = f(PLL clock input) * (PLLN / PLLM) = 16MHz * 20/2 = 160 MHz
	// f(PLL_R) = f(VCO clock) / PLLR = 160MHz/2 = 80MHz
	RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLN) | 20U << 8;
	RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM) | 1U << 4; // 000: PLLM = 1, 001: PLLM = 2, 010: PLLM = 3, 011: PLLM = 4, 100: PLLM = 5, 101: PLLM = 6, 110: PLLM = 7, 111: PLLM = 8

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR;  // 00: PLLR = 2, 01: PLLR = 4, 10: PLLR = 6, 11: PLLR = 8	
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN; // Enable Main PLL PLLCLK output 

	RCC->CR   |= RCC_CR_PLLON; 
	while((RCC->CR & RCC_CR_PLLRDY) == 0);
	
	// Select PLL selected as system clock
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL; // 00: MSI, 01:HSI, 10: HSE, 11: PLL
	
	// Wait until System Clock has been selected
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	// The maximum frequency of the AHB, the APB1 and the APB2 domains is 80 MHz.
	RCC->CFGR &= ~RCC_CFGR_HPRE;  // AHB prescaler = 1; SYSCLK not divided
	RCC->CFGR &= ~RCC_CFGR_PPRE1; // APB high-speed prescaler (APB1) = 1, HCLK not divided
	RCC->CFGR &= ~RCC_CFGR_PPRE2; // APB high-speed prescaler (APB2) = 1, HCLK not divided
	
	// RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
	// RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
	// RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP; 
	// RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLQ;	
	// RCC->PLLCFGR |= RCC_PLLCFGR_PLLPEN; // Enable Main PLL PLLSAI3CLK output enable
	// RCC->PLLCFGR |= RCC_PLLCFGR_PLLQEN; // Enable Main PLL PLL48M1CLK output enable
	
	RCC->CR &= ~RCC_CR_PLLSAI1ON;  // SAI1 PLL enable
	while ( (RCC->CR & RCC_CR_PLLSAI1ON) == RCC_CR_PLLSAI1ON );
	
	// Configure and enable PLLSAI1 clock to generate 11.294MHz 
	// 8 MHz * 24 / 17 = 11.294MHz
	// f(VCOSAI1 clock) = f(PLL clock input) *  (PLLSAI1N / PLLM)
	// PLLSAI1CLK: f(PLLSAI1_P) = f(VCOSAI1 clock) / PLLSAI1P
	// PLLUSB2CLK: f(PLLSAI1_Q) = f(VCOSAI1 clock) / PLLSAI1Q
	// PLLADC1CLK: f(PLLSAI1_R) = f(VCOSAI1 clock) / PLLSAI1R
	RCC->PLLSAI1CFGR &= ~RCC_PLLSAI1CFGR_PLLSAI1N;
	RCC->PLLSAI1CFGR |= 24U<<8;
	
	// SAI1PLL division factor for PLLSAI1CLK
	// 0: PLLSAI1P = 7, 1: PLLSAI1P = 17
	RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1P;
	RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1PEN;
	
	// SAI1PLL division factor for PLL48M2CLK (48 MHz clock)
	// RCC->PLLSAI1CFGR &= ~RCC_PLLSAI1CFGR_PLLSAI1Q;
	// RCC->PLLSAI1CFGR |= U<<21;
	// RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1QEN;
	
	// PLLSAI1 division factor for PLLADC1CLK (ADC clock)
	// 00: PLLSAI1R = 2, 01: PLLSAI1R = 4, 10: PLLSAI1R = 6, 11: PLLSAI1R = 8
	// RCC->PLLSAI1CFGR &= ~RCC_PLLSAI1CFGR_PLLSAI1R; 
	// RCC->PLLSAI1CFGR |= U<<25;
	// RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1REN;
	
	RCC->CR |= RCC_CR_PLLSAI1ON;  // SAI1 PLL enable
	while ( (RCC->CR & RCC_CR_PLLSAI1ON) == 0);
	
	// SAI1 clock source selection
	// 00: PLLSAI1 "P" clock (PLLSAI1CLK) selected as SAI1 clock
	// 01: PLLSAI2 "P" clock (PLLSAI2CLK) selected as SAI1 clock
	// 10: PLL "P" clock (PLLSAI3CLK) selected as SAI1 clock
	// 11: External input SAI1_EXTCLK selected as SAI1 clock	
	RCC->CCIPR &= ~RCC_CCIPR_SAI1SEL;

	RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;
}
	
int main(void){

	// initialization code
    
    HAL_Init();

	System_Clock_Init();        // set System Clock = 80 MHz
    SystemCoreClockUpdate();
	
    adc_init ();
    gpio_init ();
	usart2_init( );
    init_systick();

	// application run function
	main_loop() ;
}

