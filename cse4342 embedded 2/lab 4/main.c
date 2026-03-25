#include <stdint.h>
#include "stm32f446xx.h"
#include "common.h"
#include "assert.h"
#include "MCP23008.h"

volatile int windowCount, finalCount;

MCP23008_t mcp;
#define MCPCS_ON 0
#define MCP_LED_PIN BV(2)
#define MCP_BTN_PIN BV(1)

const GPIO_Pin_t 
		MCPCS = {
			.port = GPIOA,
			.pinN = 1,
	};
		/*
const GPIO_Pin_t ledBoard = {
	.port = GPIOA,
	.pinN = 5
};
	*/
const GPIO_Pin_t button = {
	.port = GPIOC,
	.pinN = 13
};

void busyDelaymS(uint32_t ms){
	volatile uint32_t i;
	
	for(; ms > 0; ms--)
		for(i = 0; i < 3195; i++)
		{}
}

void partA();
void partB();

int main(){
	windowCount = 0;
	finalCount = 0;
	
	// enable clock
	RCC->AHB1ENR |= BV(0); // PA
	RCC->AHB1ENR |= BV(1); // PB
	RCC->AHB1ENR |= BV(2); // PC
	RCC->AHB1ENR |= BV(3); // PD
	RCC->AHB1ENR |= BV(4); // PE
	RCC->AHB1ENR |= BV(5); // PF
	RCC->AHB1ENR |= BV(6); // PG
	RCC->AHB1ENR |= BV(7); // PH
	RCC->APB2ENR |= BV(14); // SYSCFG
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // SPI1
	/*
	{ // led do
		ledBoard.port->MODER &= ~	(0b11 << (2 * ledBoard.pinN));
		ledBoard.port->MODER |= 	(0b01 << (2 * ledBoard.pinN)); // DO
	}
	GPIO_setOut(&ledBoard, 1);
	*/
	
	{ // board btn
		_Static_assert(button.pinN == 13 && button.port == GPIOC, "bruh");
		
		button.port->MODER &= ~	(0b11 << (2 * button.pinN));
		button.port->MODER |= (0 << (2 * button.pinN)); // input
		GPIO_setBiasPU(&button);
	} 
	
	{
		/* SPI1
		*		A5	: SCK
		*		A6	: MISO
		*		A7	: MOSI
		*		A1	: CS
		 */

		// CS
		MCPCS.port->MODER &= ~(0b11 << (2 * MCPCS.pinN));
		MCPCS.port->MODER |= 	(0b01 << (2 * MCPCS.pinN)); // DO
		GPIO_setOut(&MCPCS, !MCPCS_ON);
		
		GPIOA->MODER &= ~(0b11 << (5*2));	// set to alternative mode
		GPIOA->MODER |= (0b10 << 	(5*2));
		GPIOA->AFR[0] &= ~(0xF << (5*4)); // set alf to 5:SCK
		GPIOA->AFR[0] |= (0x5 << 	(5*4));
		
		GPIOA->MODER &= ~(0b11 << (6*2));	// set to alternative mode
		GPIOA->MODER |= (0b10 << 	(6*2));
		GPIOA->AFR[0] &= ~(0xF << (6*4)); // set alf to 5:MISO
		GPIOA->AFR[0] |= (0x5 << 	(6*4));
		
		GPIOA->MODER &= ~(0b11 << (7*2));
		GPIOA->MODER |= (0b10 << 	(7*2));
		GPIOA->AFR[0] &= ~(0xF << (7*4)); // set alf to 5:MOSI
		GPIOA->AFR[0] |= (0x5 << 	(7*4));
		
		SPI1->CR1 &= ~(1 << 6); // disable spi
		SPI1->CR1 &= ~(1 << 7); // MSB first
		SPI1->CR1 &= ~(1 << 11); // 8 bit data
		SPI1->CR1 &= ~(0b111 << 3); // set baud
		SPI1->CR1 |= 	(4 << 3); // set baud
		SPI1->CR1 &= ~(1 << 13); // disable CRC
		SPI1->CR1 &= ~(1 << 15); // normal 2 line spi 
		SPI1->CR1 &= ~(1 << 10); // trx 
		SPI1->CR1 |= (0b11 << 8); // software slave management
		SPI1->CR1 |= 	(1 << 2); // SPI master
		SPI1->CR1 &= ~(1 << 1); // SPI POL
		SPI1->CR1	&= ~(1 << 0); // SPI PHA
		SPI1->CR1 |= 	(1 << 6); // enable spi
	}
	
	mcp = (MCP23008_t){
		.cs = MCPCS,
		.spi= SPI1,
		.addr_ext = 0b11,
	};
	
	__enable_irq();
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
	delaymS(1e3);
	
	MCP23008_write(&mcp, REG_IODIR, MCP_BTN_PIN); // button as input
	MCP23008_write(&mcp, REG_GPPU, MCP_BTN_PIN); 	// PU on button
	
	while(1) {
		partA();
		//partB();
		//static uint8_t out = 0;
		//out = out ? 0 : MCP_LED_PIN;
		//uint8_t tx[] = {1,2,3};
		//SPI_transfer8(SPI1, tx, 0, sizeof(tx), &MCPCS);
		//MCP23008_write(&mcp, REG_GPIO, out);
		delaymS(500);
	}
}

void partA() {
	uint8_t gpio_old, gpio_new;
	gpio_old = gpio_new = MCP23008_read(&mcp, REG_GPIO);
	
	while(1) {
		gpio_new = MCP23008_read(&mcp, REG_GPIO);
		if(gpio_old & MCP_BTN_PIN)
			if(!(gpio_new & MCP_BTN_PIN)) {
			
				gpio_new ^= MCP_LED_PIN;
				MCP23008_write(&mcp, REG_GPIO, gpio_new);
				
				delaymS(15);
			}
		
		gpio_old = gpio_new;
	}
}

void partB() {
	// pa9 as interrupt
	GPIO_Pin_t p = {
		.port = GPIOA,
		.pinN = 9,
	};
	//GPIO_setBiasPU(&p);
	
	GPIOA->MODER &= ~(0b11 << (9*2)); // as input
	GPIOA->MODER |= (0 << (9*2));
	
	SYSCFG->EXTICR[2] &= ~(0xF << 4);	// enable irq
	SYSCFG->EXTICR[2] |= 0 << 4;
	
	EXTI->IMR |= BV(9); // unmask
	EXTI->RTSR |= BV(9); // trigger on rising
	
	{
		uint8_t buf = MCP23008_read(&mcp, REG_IOCON);
		buf |= BV(1); // irq is active high
		buf &= ~BV(2); // irq actively driven
		MCP23008_write(&mcp, REG_IOCON, buf);
	}
	
	MCP23008_write(&mcp, REG_GPINTEN, MCP_BTN_PIN);
	MCP23008_write(&mcp, REG_DEFVAL, MCP_BTN_PIN);
	MCP23008_write(&mcp, REG_INTCON, MCP_BTN_PIN);
	
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	while(1);
}


void EXTI9_5_IRQHandler() {
	//GPIO_toggleOut(&ledBoard);
	uint8_t mcp_irq_pr = MCP23008_read(&mcp, REG_INTF);
	if(mcp_irq_pr & MCP_BTN_PIN) {
		uint8_t buf = MCP23008_read(&mcp, REG_GPIO);
		buf ^= MCP_LED_PIN;
		MCP23008_write(&mcp, REG_GPIO, buf);
	}

	MCP23008_read(&mcp, REG_INTF);
	delaymS(10);
	
	EXTI->PR = BV(9);
}

void SysTick_Handler(void) {
	tick++;
}
