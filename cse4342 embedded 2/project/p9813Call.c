#include "p9813.h"

static void P9813_DelayCycles(volatile uint32_t count)
{
    while (count--)
    {
        __NOP();
    }
}

static void P9813_CLK_High(void)
{
    GPIOB->BSRR = (1 << 0);
}

static void P9813_CLK_Low(void)
{
    GPIOB->BSRR = (1 << 16);
}

static void P9813_DATA_High(void)
{
    GPIOB->BSRR = (1 << 1);
}

static void P9813_DATA_Low(void)
{
    GPIOB->BSRR = (1 << 17);
}

static void P9813_ClockPulse(void)
{
    P9813_CLK_Low();
    P9813_DelayCycles(200);
    P9813_CLK_High();
    P9813_DelayCycles(200);
}

static void P9813_SendByte(uint8_t value)
{
    uint8_t bit;

    for (bit = 0; bit < 8; bit++) // Sends bit by bit
    {
        if ((value & 0x80) != 0) // Checks what the MsB is
            P9813_DATA_High(); // If its '1', send a high data pulse
        else
            P9813_DATA_Low(); // If it is '0', send a low data pulse

        P9813_DelayCycles(100);
        P9813_ClockPulse(); // Pulses clock for that bit
        value <<= 1; // Shifts the value to the left for the MsB
    }
}

static void P9813_SendColor(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t rr = (uint8_t)(0xFF - red);
    uint8_t gr = (uint8_t)(0xFF - green);
    uint8_t br = (uint8_t)(0xFF - blue);

    uint8_t flags = 0xC0; // Sets upper 2 bits of the flags to '11'

    if ((br & 0x80) == 0) { flags |= 0x20; } // Sets the upper bits for the prefix of the p9813 communication
    if ((br & 0x40) == 0) { flags |= 0x10; } // For some reason the p9813 is inverted, so it sets the upper bits to the opposite 
    if ((gr & 0x80) == 0) { flags |= 0x08; } // (eg. 1 is turned to 0)
    if ((gr & 0x40) == 0) { flags |= 0x04; }
    if ((rr & 0x80) == 0) { flags |= 0x02; }
    if ((rr & 0x40) == 0) { flags |= 0x01; }

    P9813_SendByte(flags); // Send prefix
    P9813_SendByte(br); // send blue value
    P9813_SendByte(gr); // send green value
    P9813_SendByte(rr); // send red value
}

void initLED_Driver(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;

    GPIOB->MODER &= ~((3 << 0) | (3 << (1 * 2))); // PB0, PB1 set as outputs
    GPIOB->MODER |=  (1 << 0) | (1 << (1 * 2));

    GPIOB->OTYPER &= ~((1 << 0) | (1 << 1)); // Push-Pull

    GPIOB->OSPEEDR &= ~((3 << 0) | (3 << (1 * 2))); // Set to medium speed
    GPIOB->OSPEEDR |=  (1 << 0) | (1 << (1 * 2));

    GPIOB->PUPDR &= ~((3 << 0) | (3 << (1 * 2))); // No pull up/down

    P9813_CLK_Low(); // Pull both clock and data low
    P9813_DATA_Low();

    Set_Color_RGB(255, 255, 255);   // Turn off initially
}

void Set_Color_RGB(uint8_t red, uint8_t green, uint8_t blue)
{
    P9813_SendByte(0x00); // Start condition is 32 '0's
    P9813_SendByte(0x00);
    P9813_SendByte(0x00);
    P9813_SendByte(0x00);

    P9813_SendColor(red, green, blue);

    P9813_SendByte(0x00); // End condition is 32 '0's
    P9813_SendByte(0x00);
    P9813_SendByte(0x00);
    P9813_SendByte(0x00);
}

static void Delay(volatile uint32_t count)
{
    while (count--)
    {
        __NOP();
    }
}

int main_womp(void)
{
    initLED_Driver();

    while (1)
    {
        Set_Color_RGB(255U, 255U, 255U);  //Off
        Delay(3000000);

        Set_Color_RGB(0U, 255U, 255U);    // Red 
        Delay(3000000);

        Set_Color_RGB(255U, 0U, 255U);    // Green
        Delay(3000000);

        Set_Color_RGB(255U, 255U, 0U);    // Blue
        Delay(3000000);

				Set_Color_RGB(0U,   180U, 255U);  // Orange        
        Delay(3000000);
			
				Set_Color_RGB(0U,   0U,   255U);  // Yellow
				Delay(3000000);
			
				Set_Color_RGB(0U,   255U, 0U);    // Purple
				Delay(3000000);
			
				Set_Color_RGB(0U, 0U, 0U);        // White
				Delay(3000000);
    }
}