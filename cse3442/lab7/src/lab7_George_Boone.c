/**
 *  cse3442 fall2024 Lab7
 *  George Boone
 *  1002055713
 *
 * the lab requirements say this is "lab1" but the files titled "lab7". assuming "lab1" was a typo.
 *
 * - interfacing the MCP23S08
 *
 * IO
 *  PA2-5   SSI0 for SPI with the MCP
 *  PA6     interrupt handler for the MCP
 *
 *  - SPI SS is manually controlled
 *  - non standard SPI operations to meet MCP requirements
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../tm4c123gh6pm.h"

#include "uart0.h"
#include "MCP23S08_support.h"

#define _BV(X)  (1 << X)
#define SCK_M     _BV(2)
#define SS_M      _BV(3)
#define DI_M      _BV(4)
#define DO_M      _BV(5)

#define SS_BB       (*((volatile uint32_t *)(3 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define SS_ON       0
#define SS_OFF      1

void SSI0_SPI_init_master();
uint8_t SSI0_SPI_master_transfer(uint8_t);
void    SSI0_SPI_master_transfer_arr(uint8_t * tx, uint8_t * rx, size_t len);

uint8_t SSI0_SPI_MCP23S08_READ(uint8_t addr);
void    SSI0_SPI_MCP23S08_WRITE(uint8_t addr, uint8_t val);

bool BTN_PRESSED = false;

int main(void)
{
    //init system clock to 40MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    //----------------init uart--------------------------
    initUart0();
    setUart0BaudRate(115200, 40e6);
    putsUart0("lab7 start\n\r");


    //---------------init ssi0 as spi--------------------
    SSI0_SPI_init_master();
    SS_BB = SS_OFF;
    _delay_cycles(10e6);

    //------------MCP to lab requirements----------------
//    {
//        uint8_t tx[1] = {0x4};
//        SSI0_SPI_master_transfer_arr(tx, tx, sizeof(tx));
//    }

    /*GPIO
     * input    : 7
     * output   : 4, 5
     */
    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_IODIR, _BV(7));
    uint8_t v = SSI0_SPI_MCP23S08_READ(MCP23S08_ADDR_IODIR);
    // general configuration
    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_INTCON,   0x06);      // interrupt line active high, enable hardware addressing

    // interrupt for pin 7
    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_GPINTEN,  _BV(7));    // enable interrupt for GP7
    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_INTCON,   0x00);      // edge detection


    //-----------init PA6 as MCP interrupt---------------
    SYSCTL_RCGCGPIO_R   |=  SYSCTL_RCGCGPIO_R0; // sys clock to port A
    _delay_cycles(3);

    // GPIO setup
    GPIO_PORTA_DIR_R    &=  ~_BV(6);            // as input
    GPIO_PORTA_DEN_R    |=  _BV(6);             // as digital

    // set up interrupt
    GPIO_PORTA_IM_R     |=  _BV(6);             // disable port A interrupts
    GPIO_PORTA_IS_R     |=  _BV(6);             // level sensitive
    GPIO_PORTA_IEV_R    |=  _BV(6);             // active high
    GPIO_PORTA_IM_R     |=  _BV(6);             // enable port A interrupts


    //---------------main logic--------------------------

    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_GPIO, _BV(4)); // turn on red, off green led

    NVIC_EN0_R |= 1 << (INT_GPIOA - 16);    // enable port A interrupts on the NVIC
    _delay_cycles(40e6);                    // wait 1s

    while(!BTN_PRESSED)
        ;

    SSI0_SPI_MCP23S08_WRITE(MCP23S08_ADDR_GPIO, _BV(5)); // turn off red, on green led

    while(true)
        ;


	return 0;
}

void SSI0_SPI_init_master(){
    // uP.965 , 15.4
    // master , active low ss , mode 1 , 8 bit
    // sck 2MHz
    /** io  : purpose
     * PA2  : SCK
     * PA3  : SS        THIS IS MANUALY DONE
     * PA4  : DI
     * PA5  : DO
     */
    SYSCTL_RCGCSSI_R    |= SYSCTL_RCGCSSI_R0;
    SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R0;    // sys clock to port A
    _delay_cycles(3);

    // SS pin gets special treatment in this case since its manually controlled
    GPIO_PORTA_DIR_R    |= SS_M;
    GPIO_PORTA_DEN_R    |= SS_M;
    SS_BB = SS_OFF;

    GPIO_PORTA_AFSEL_R  |= SCK_M | DI_M | DO_M;         // use alt function
    // set ssi as alt function
    GPIO_PORTA_PCTL_R   &= ~(GPIO_PCTL_PA2_M | GPIO_PCTL_PA4_M | GPIO_PCTL_PA5_M);
    GPIO_PORTA_PCTL_R   |= GPIO_PCTL_PA2_SSI0CLK | GPIO_PCTL_PA3_SSI0FSS | GPIO_PCTL_PA4_SSI0RX | GPIO_PCTL_PA5_SSI0TX;
    GPIO_PORTA_DEN_R    |= SCK_M | SS_M | DI_M | DO_M;  // enable digital function
    GPIO_PORTA_PUR_R    |= SCK_M;                       // pull up on clk line when using idle high, required by uP doc.

    SSI0_CR1_R  &=  ~SSI_CR1_SSE;                       // disable to safely configure
    SSI0_CR1_R  =   0;                                  // operate as master
    SSI0_CC_R   =   SSI_CC_CS_SYSPLL;                   // use system clock (40MHz expected)
    SSI0_CPSR_R =   20;                                 // clk div by 20 for 2MHz
    SSI0_CR0_R  &=  ~(SSI_CR0_SCR_M | SSI_CR0_FRF_M | SSI_CR0_DSS_M);   // bit rate same as clk , freescale spi format
    SSI0_CR0_R  |=  SSI_CR0_SPO | SSI_CR0_SPH;          // spi 11 : clk idle high , read on second clk edge
    SSI0_CR0_R  |=  SSI_CR0_DSS_8;                      // 8 bit
    SSI0_CR1_R  |=  SSI_CR1_SSE;                        // enable
}

uint8_t SSI0_SPI_master_transfer(uint8_t tx){
    SS_BB = SS_ON;

    SSI0_DR_R = tx;

    // while SSI is busy
    while(SSI0_SR_R & SSI_SR_BSY)
        ;

    SS_BB = SS_OFF;

    return SSI0_DR_R;
}

void SSI0_SPI_master_transfer_arr(uint8_t * tx, uint8_t * rx, size_t len){
    /* MCP23S08 datasheet : https://ww1.microchip.com/downloads/en/devicedoc/21919e.pdf
     * note figure 1-5 (page 8). theres a extra bit between bytes...
     *  this is configured to transmit 9 bits along with extra logic to convert it to 8 bits
     */

    uint8_t former_cr0 = SSI0_CR0_R;
    // SSI0 to 9 bit mode
    SSI0_CR1_R  &=  ~SSI_CR1_SSE;       // disable to safely configure
    SSI0_CR0_R  &=  ~(SSI_CR0_DSS_M);   // clear bit mode
    SSI0_CR0_R  |=  SSI_CR0_DSS_9;      // 9 bit mode
    SSI0_CR1_R  |=  SSI_CR1_SSE;        // enable

    SS_BB = SS_ON;

    // write one read one approach
    size_t i;
    for(i = 0; i < len; i++){
        uint16_t tx_b = tx[i] << 1;
        SSI0_DR_R = ((uint16_t)tx[i]) << 1;

        while(SSI0_SR_R & SSI_SR_BSY);

        if(rx)
            rx[i] = SSI0_DR_R >> 1;
        else
            SSI0_DR_R;//read to clear buffer
    }

    SS_BB = SS_OFF;

    // revert local SSI0 CR0 changes
    SSI0_CR1_R  &=  ~SSI_CR1_SSE;       // disable to safely configure
    SSI0_CR0_R  =   former_cr0;
    SSI0_CR1_R  |=  SSI_CR1_SSE;        // enable
}

uint8_t SSI0_SPI_MCP23S08_READ(uint8_t addr){
    uint8_t txrx[2];

    txrx[0] = MCP23S08_OPC;
    txrx[1] = addr;
    SSI0_SPI_master_transfer_arr(txrx, NULL, 2);

    txrx[0] = MCP23S08_OPC | MCP23S08_OPC_RW_M;
    txrx[1] = addr;

    SSI0_SPI_master_transfer_arr(txrx, txrx, 2);

    SS_BB = SS_ON;
    _delay_cycles(3);
    SS_BB = SS_OFF;
    _delay_cycles(3);
    SS_BB = SS_ON;
    _delay_cycles(3);
    SS_BB = SS_OFF;
    _delay_cycles(3);

    return txrx[1];
}

void SSI0_SPI_MCP23S08_WRITE(uint8_t addr, uint8_t val){
    uint8_t tx[2] = {
            MCP23S08_OPC,
            addr
        };

    SSI0_SPI_master_transfer_arr(tx, NULL, 2);

    tx[1] = val;
    SSI0_SPI_master_transfer_arr(tx, NULL, 2);

    SS_BB = SS_ON;
    _delay_cycles(3);
    SS_BB = SS_OFF;
    _delay_cycles(3);
    SS_BB = SS_ON;
    _delay_cycles(3);
    SS_BB = SS_OFF;
    _delay_cycles(3);
}

void intr_MCP23S08(){
    // interrupt flags
    uint8_t flags = SSI0_SPI_MCP23S08_READ(MCP23S08_ADDR_INTF);

    // clear interrupt flags on the MCP
    SSI0_SPI_MCP23S08_READ(MCP23S08_ADDR_INTCAP);

    // no need to clear local interrupt flags as its level detected

    putsUart0("mcp interrupt!  ");
    {
        char str[10];
        snprintf(str, sizeof(str), "x%02x", flags);
        putsUart0(str);
    }

    if(flags & _BV(7)){
        BTN_PRESSED = true;
        putsUart0("   button pressed");
    } else {
        putsUart0("   not button pressed");
    }
    putsUart0("\n\r");
}
