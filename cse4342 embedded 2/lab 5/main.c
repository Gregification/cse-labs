#include <stdint.h>
#include "stm32f446xx.h"
#include "common.h"
#include "assert.h"


// ----------- MCP23008 Registers -----------
#define MCP23008_ADDR   0x20u   // 7-bit I2C address (A2..A0 = 000)
#define MCP_IODIR       0x00u
#define MCP_IPOL        0x01u
#define MCP_GPINTEN     0x02u
#define MCP_DEFVAL      0x03u
#define MCP_INTCON      0x04u
#define MCP_IOCON       0x05u
#define MCP_GPPU        0x06u
#define MCP_INTF        0x07u
#define MCP_INTCAP      0x08u
#define MCP_GPIO        0x09u
#define MCP_OLAT        0x0Au

#define MCP_SENSE_PIN		BV(0)

// ----------- GPIOB init for I2C1 on PB8/PB9 -----------
void gpio_i2c1_pb8_pb9_init(void);

// ----------- I2C1 init: Standard Mode 100 kHz @ PCLK1 = 16 MHz -----------
void i2c1_init_100k_16mhz(void);

// Generate START and send 7-bit address + R/W bit
// dir_write: 1 = write, 0 = read
int i2c1_start_and_addr7(uint8_t addr7, uint8_t dir_write);

void i2c1_stop(void);

// Write one register, one byte
int i2c1_write_reg1(uint8_t addr7, uint8_t reg, uint8_t data);
	
uint8_t i2c1_read_reg1(uint8_t addr7, uint8_t reg);


const GPIO_Pin_t ledBoard = {
	.port = GPIOA,
	.pinN = 5
};
const GPIO_Pin_t button = {
	.port = GPIOC,
	.pinN = 13
};
const GPIO_Pin_t MCP_RESET = {
	.port = GPIOA,
	.pinN = 9
};

// ----------- MCP23008 setup
void mcp23008_init(void);
	

int main(){
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
	
	{ // led do
		_Static_assert(ledBoard.pinN == 5 && ledBoard.port == GPIOA, "bruh");
		ledBoard.port->MODER &= ~	(0b11 << (2 * ledBoard.pinN));
		ledBoard.port->MODER |= 	(0b01 << (2 * ledBoard.pinN)); // DO
	}
	GPIO_setOut(&ledBoard, 1);
	
	{ 
		MCP_RESET.port->MODER &= ~	(0b11 << (2 * MCP_RESET.pinN));
		MCP_RESET.port->MODER |= 	(0b01 << (2 * MCP_RESET.pinN)); // DO
	}
	GPIO_setOut(&MCP_RESET, 1);
	
	{ // board btn
		_Static_assert(button.pinN == 13 && button.port == GPIOC, "bruh");
		
		button.port->MODER &= ~	(0b11 << (2 * button.pinN));
		button.port->MODER |= (0 << (2 * button.pinN)); // input
		GPIO_setBiasPU(&button);
	} 
	
	__enable_irq();
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
	delaymS(10);
	
	// 2) Initialize GPIOB pins PB8/PB9 for I2C1 AF4
	gpio_i2c1_pb8_pb9_init();

	// 3) Initialize I2C1 for 100 kHz (SM) with PCLK1 = 16 MHz
	i2c1_init_100k_16mhz();

	// 4) Initialize MCP23008
	mcp23008_init();

	
	static uint8_t mask = 0;
	static bool latch = false;
	
	while (1)
	{
		//ADD YOUR CODE HERE 
		// TODO
		{
			if(i2c1_read_reg1(MCP23008_ADDR, MCP_GPIO) & MCP_SENSE_PIN) {
				GPIO_setOut(&ledBoard, 1);
				latch = false;
				
			} else {
				if(latch) continue;
				latch = true;
				
				if(mask == 0xF)
					mask = 0;
				else
					mask = (mask << 1) + 1;
				
				i2c1_write_reg1(MCP23008_ADDR, MCP_GPIO, mask<<1);
				
				delaymS(20);
			}
		}
	}
	
}

void gpio_i2c1_pb8_pb9_init(void)
{
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // PB8 = I2C1_SCL (AF4), PB9 = I2C1_SDA (AF4)
    // MODER: Alt Function (10)
    GPIOB->MODER &= ~((3u<<(8*2)) | (3u<<(9*2)));
    GPIOB->MODER |=  ((2u<<(8*2)) | (2u<<(9*2)));

    // AFRH: AF4 on PB8/PB9
    GPIOB->AFR[1] &= ~((0xFu<<((8-8)*4)) | (0xFu<<((9-8)*4)));
    GPIOB->AFR[1] |=  ((4u<<((8-8)*4)) | (4u<<((9-8)*4)));

    // Open-drain outputs for I2C
    //**************************ADD YOUR CODE HERE********************************************************
		GPIOB->OTYPER |= BV(8) | BV(9);

    // Pull-ups (optional)
    GPIOB->PUPDR &= ~((3u<<(8*2)) | (3u<<(9*2)));
    GPIOB->PUPDR |=  ((1u<<(8*2)) | (1u<<(9*2)));  // 01 = pull-up

    // High speed for cleaner I2C edges
    GPIOB->OSPEEDR |= (3u<<(8*2)) | (3u<<(9*2));   // 11 = very high
}

void i2c1_init_100k_16mhz(void)
{
	// Enable I2C1 clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	// Reset then release reset
	RCC->APB1RSTR |=  RCC_APB1RSTR_I2C1RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

	// Make sure I2C is disabled before configuring
	I2C1->CR1 = 0;
	I2C1->CR2 = 0;

	// CR2: PCLK1 frequency in MHz
	I2C1->CR2 = (16u & I2C_CR2_FREQ);

	// CCR: Standard mode (SM, 100 kHz): CCR = Fpclk1/(2*Fscl) = 16MHz/(2*100k) = 80
	//******************************ADD YOUR CODE HERE*********************************************************
	I2C1->CCR = 80;
	I2C1->CCR &= ~BV(15); // sm mode
	I2C1->CCR &= ~BV(14); // ignore
	
	// TRISE: in SM, TRISE = Fpclk1(MHz) + 1 = 17
	I2C1->TRISE = 17u;

	// Enable peripheral
	I2C1->CR1 |= I2C_CR1_PE;
}

int i2c1_start_and_addr7(uint8_t addr7, uint8_t dir_write)
{
    // Generate START
    I2C1->CR1 |= I2C_CR1_START;
    // Wait for SB (start bit set)
    while (!(I2C1->SR1 & I2C_SR1_SB)) { }

    // Send 7-bit address + R/W bit
    uint8_t addr_byte = (addr7 << 1) | (dir_write ? 0u : 1u);
    I2C1->DR = addr_byte;

    // Wait for ADDR (address sent/ack)
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) { }

    // Clear ADDR by reading SR1 then SR2
    (void)I2C1->SR1; (void)I2C1->SR2;
    return 0;
}

void i2c1_stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
}

int i2c1_write_reg1(uint8_t addr7, uint8_t reg, uint8_t data)
{
    i2c1_start_and_addr7(addr7, 1);  // write

    // Send register address
    while (!(I2C1->SR1 & I2C_SR1_TXE)) { }
    I2C1->DR = reg;

    // Wait for TXE/BTF before sending data
    while (!(I2C1->SR1 & I2C_SR1_TXE)) { }
    I2C1->DR = data;

    // Wait for byte transfer finished
    while (!(I2C1->SR1 & I2C_SR1_BTF)) { }

    // STOP
    i2c1_stop();
    return 0;
}

uint8_t i2c1_read_reg1(uint8_t addr7, uint8_t reg)
{
    //****************************ADD YOUR CODE HERE****************************************************************
		uint8_t ret;
	
		// send write to register
		{ 
			i2c1_start_and_addr7(addr7, 1); 

			// Send register address
			while (!(I2C1->SR1 & I2C_SR1_TXE)) { }
			I2C1->DR = reg;
			
			// Wait for byte transfer finished
			while (!(I2C1->SR1 & I2C_SR1_BTF)) { }
		}
		
		// repeated start with reading
		i2c1_start_and_addr7(addr7, 0);
		
		{
			
			I2C1->CR1 &= ~I2C_CR1_ACK; // disable acks
			
			i2c1_stop(); // stop after current byte transfre finished
			
			// wait for transfer to complete
			while (!(I2C1->SR1 & I2C_SR1_RXNE)) { }
			
			ret = I2C1->DR;
			
			I2C1->CR1 |= I2C_CR1_ACK; // enable acks
		}

    return ret;
}

// ----------- MCP23008 setup
void mcp23008_init(void)
{
	//ADD YOUR CODE HERE
	
	// reset device
	GPIO_setOut(&MCP_RESET, 1);
	delaymS(50);
	GPIO_setOut(&MCP_RESET, 0);
	delaymS(50);
	GPIO_setOut(&MCP_RESET, 1);
	delaymS(50);
	
	i2c1_write_reg1(MCP23008_ADDR, MCP_IODIR, MCP_SENSE_PIN); // dio input
	i2c1_write_reg1(MCP23008_ADDR, MCP_GPPU, MCP_SENSE_PIN); // dio PU
}

void SysTick_Handler(void) {
	tick++;
}