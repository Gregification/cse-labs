// I2C0 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// I2C devices on I2C bus 0 with 2kohm pullups on SDA and SCL (PB3 and PB2)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "i2c0.h"

// Pins
#define I2C0SCL PORTB,2
#define I2C0SDA PORTB,3

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initI2c0(void)
{
    // Enable clocks
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;
    _delay_cycles(3);
    enablePort(PORTB);

    // Configure I2C
    selectPinPushPullOutput(I2C0SCL);
    setPinAuxFunction(I2C0SCL, GPIO_PCTL_PB2_I2C0SCL);
    selectPinOpenDrainOutput(I2C0SDA);
    setPinAuxFunction(I2C0SDA, GPIO_PCTL_PB3_I2C0SDA);

    // Configure I2C0 peripheral
    I2C0_MCR_R = 0;                                     // disable to program
    I2C0_MTPR_R = 19;                                   // (40MHz/2) / (6+4) / (19+1) = 100kbps
    I2C0_MCR_R = I2C_MCR_MFE;                           // master
    I2C0_MCS_R = I2C_MCS_STOP;
}

// For simple devices with a single internal register
void writeI2c0Data(uint8_t add, uint8_t data)
{
    I2C0_MSA_R = add << 1 | 0; // add:r/~w=0
    I2C0_MDR_R = data;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
}

uint8_t readI2c0Data(uint8_t add)
{
    I2C0_MSA_R = (add << 1) | 1; // add:r/~w=1
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    return I2C0_MDR_R;
}

// For devices with multiple registers
void writeI2c0Register(uint8_t add, uint8_t reg, uint8_t data)
{
    // send address and register
    I2C0_MSA_R = add << 1 | 0; // add:r/~w=0
    I2C0_MDR_R = reg;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);

    // write data to register
    I2C0_MDR_R = data;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_STOP;
    while (!(I2C0_MRIS_R & I2C_MRIS_RIS));
}

void writeI2c0Registers(uint8_t add, uint8_t reg, const uint8_t data[], uint8_t size)
{
    uint8_t i;
    // send address and register
    I2C0_MSA_R = add << 1 | 0; // add:r/~w=0
    I2C0_MDR_R = reg;
    if (size == 0)
    {
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    }
    else
    {
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        // first size-1 bytes
        for (i = 0; i < size-1; i++)
        {
            I2C0_MDR_R = data[i];
            I2C0_MICR_R = I2C_MICR_IC;
            I2C0_MCS_R = I2C_MCS_RUN;
            while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        }
        // last byte
        I2C0_MDR_R = data[size-1];
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_STOP;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    }
}

uint8_t readI2c0Register(uint8_t add, uint8_t reg)
{
    // set internal register counter in device
    I2C0_MSA_R = add << 1 | 0; // add:r/~w=0
    I2C0_MDR_R = reg;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);

    // read data from register
    I2C0_MSA_R = (add << 1) | 1; // add:r/~w=1
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    return I2C0_MDR_R;
}

void readI2c0Registers(uint8_t add, uint8_t reg, uint8_t data[], uint8_t size)
{
    uint8_t i = 0;
    // send address and register number
    I2C0_MSA_R = add << 1; // add:r/~w=0
    I2C0_MDR_R = reg;
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);

    if (size == 1)
    {
        // add and read one byte
        I2C0_MSA_R = (add << 1) | 1; // add:r/~w=1
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        data[i++] = I2C0_MDR_R;
    }
    else if (size > 1)
    {
        // add and first byte of read with ack
        I2C0_MSA_R = (add << 1) | 1; // add:r/~w=1
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_ACK;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        data[i++] = I2C0_MDR_R;
        // read size-2 bytes with ack
        while (i < size-1)
        {
            I2C0_MICR_R = I2C_MICR_IC;
            I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_ACK;
            while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
            data[i++] = I2C0_MDR_R;
        }
        // last byte of read with nack
        I2C0_MICR_R = I2C_MICR_IC;
        I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_STOP;
        while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
        data[i++] = I2C0_MDR_R;
    }
}

bool pollI2c0Address(uint8_t add)
{
    I2C0_MSA_R = (add << 1) | 1; // add:r/~w=1
    I2C0_MICR_R = I2C_MICR_IC;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;
    while ((I2C0_MRIS_R & I2C_MRIS_RIS) == 0);
    return !(I2C0_MCS_R & I2C_MCS_ERROR);
}

bool isI2c0Error(void)
{
    return (I2C0_MCS_R & I2C_MCS_ERROR);
}


