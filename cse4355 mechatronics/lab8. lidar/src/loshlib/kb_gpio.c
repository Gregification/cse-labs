// Keyboard Library
// Jason Losh

// Hook in debounceIsr to TIMER1A IVT entry
// Hook in keyPressIsr to GPIOA and GPIOE IVT entries

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// 4x4 Keyboard
//   Column 0-3 open drain outputs on PB0, PB1, PB4, PA6
//   Rows 0-3 inputs with pull-ups on PE1, PE2, PE3, PA7
//   To locate a key (r, c), the column c is driven low so the row r reads low

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "kb.h"
#include "nvic.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// Pins
#define COL0 PORTB,0
#define COL1 PORTB,1
#define COL2 PORTB,4
#define COL3 PORTA,6
#define ROW0 PORTE,1
#define ROW1 PORTE,2
#define ROW2 PORTE,3
#define ROW3 PORTA,7

// Keyboard variables
#define KB_BUFFER_LENGTH 16
#define KB_NO_KEY -1
char keyboardBuffer[KB_BUFFER_LENGTH];
uint8_t keyboardReadIndex = 0;
uint8_t keyboardWriteIndex = 0;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initKb()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    _delay_cycles(3);
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTE);

    // Configure keyboard
    // Columns 0-3 with open-drain outpus connected to PB0, PB1, PB4, PA6
    // Rows 0-3 with pull-ups connected to PE1, PE2, PE3, PA7
    selectPinOpenDrainOutput(COL0);
    selectPinOpenDrainOutput(COL1);
    selectPinOpenDrainOutput(COL2);
    selectPinOpenDrainOutput(COL3);
    selectPinDigitalInput(ROW0);
    selectPinDigitalInput(ROW1);
    selectPinDigitalInput(ROW2);
    selectPinDigitalInput(ROW3);
    enablePinPullup(ROW0);
    enablePinPullup(ROW1);
    enablePinPullup(ROW2);
    enablePinPullup(ROW3);

    // Configure falling edge interrupts on row inputs
    // (edge mode, single edge, falling edge, clear any interrupts, turn on)
    selectPinInterruptFallingEdge(ROW0);
    selectPinInterruptFallingEdge(ROW1);
    selectPinInterruptFallingEdge(ROW2);
    selectPinInterruptFallingEdge(ROW3);

    clearPinInterrupt(ROW0);
    clearPinInterrupt(ROW1);
    clearPinInterrupt(ROW2);
    clearPinInterrupt(ROW3);
    enableNvicInterrupt(INT_GPIOA);                  // turn-on interrupt 16 (GPIOA)
    enableNvicInterrupt(INT_GPIOE);                  // turn-on interrupt 20 (GPIOE)

    // Configure Timer 1 for keyboard service
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 200000;                         // set load value to 2e5 for 200 Hz interrupt rate
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    enableNvicInterrupt(INT_TIMER1A);                // turn-on interrupt 37 (TIMER1A)

    // Configure NVIC to require debounce before key press detection
    disablePinInterrupt(ROW0);                       // turn-off key press interrupts
    disablePinInterrupt(ROW1);
    disablePinInterrupt(ROW2);
    disablePinInterrupt(ROW3);
    TIMER1_IMR_R |= TIMER_IMR_TATOIM;                 // turn-on debounce interrupt
}

// Non-blocking function called to drive a selected column low for readout
void setKeyboardColumn(int8_t col)
{
    setPinValue(COL0, col != 0);
    setPinValue(COL1, col != 1);
    setPinValue(COL2, col != 2);
    setPinValue(COL3, col != 3);
    __asm(" NOP \n NOP \n NOP \n NOP \n");
}

// Non-blocking function called to drive all selected column low for readout
void setKeyboardAllColumns()
{
    //COL0 = COL1 = COL2 = COL3 = 0;
    setPinValue(COL0, 0);
    setPinValue(COL1, 0);
    setPinValue(COL2, 0);
    setPinValue(COL3, 0);
    __asm(" NOP \n NOP \n NOP \n NOP \n");
}

// Non-blocking function called to determine is a key is pressed in the selected column
int8_t getKeyboardRow()
{
    int8_t row = KB_NO_KEY;
    if (!getPinValue(ROW0)) row = 0;
    if (!getPinValue(ROW1)) row = 1;
    if (!getPinValue(ROW2)) row = 2;
    if (!getPinValue(ROW3)) row = 3;
    return row;
}

// Non-blocking function called by the keyboard ISR to determine if a key is pressed
int8_t getKeyboardScanCode()
{
    uint8_t col = 0;
    int8_t row;
    int8_t code = KB_NO_KEY;
    bool found = false;
    while (!found && (col < 4))
    {
        setKeyboardColumn(col);
        row = getKeyboardRow();
        found = row != KB_NO_KEY;
        if (found)
            code = row << 2 | col;
        else
            col++;
    }
    return code;
}

// Key press detection interrupt
void keyPressIsr()
{
    // Handle key press
    bool full;
    int8_t code;
    code = getKeyboardScanCode();
    if (code != KB_NO_KEY)
    {
        full = ((keyboardWriteIndex+1) % KB_BUFFER_LENGTH) == keyboardReadIndex;
        if (!full)
        {
            keyboardBuffer[keyboardWriteIndex] = code;
            keyboardWriteIndex = (keyboardWriteIndex + 1) % KB_BUFFER_LENGTH;
        }
        disablePinInterrupt(ROW0);                        // turn-off key press interrupts
        disablePinInterrupt(ROW1);
        disablePinInterrupt(ROW2);
        disablePinInterrupt(ROW3);
        TIMER1_IMR_R |= TIMER_IMR_TATOIM;                  // turn-on debounce interrupt
    }
}

// 5ms timer interrupt used for debouncing
void debounceIsr()
{
    static uint8_t debounceCount = 0;
    setKeyboardAllColumns();
    if (getKeyboardRow() != KB_NO_KEY)
        debounceCount = 0;
    else
    {
        debounceCount ++;
        if (debounceCount == 10)
        {
            debounceCount = 0;                                  // zero for next debounce
            clearPinInterrupt(ROW0);
            clearPinInterrupt(ROW1);
            clearPinInterrupt(ROW2);
            clearPinInterrupt(ROW3);
            enablePinInterrupt(ROW0);                           // turn-on key press interrupts
            enablePinInterrupt(ROW1);
            enablePinInterrupt(ROW2);
            enablePinInterrupt(ROW3);
            TIMER1_IMR_R &= ~TIMER_IMR_TATOIM;                  // turn-off debounce interrupt
        }
    }
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;
}

// Non-blocking function called by the user to determine if a key is present in the buffer
bool kbhit()
{
    return (keyboardReadIndex != keyboardWriteIndex);
}

// Blocking function called by the user to get a keyboard character
char getKey()
{
    const char keyCap[17] = {"123A456B789C*0#D"};
    while (!kbhit());
    uint8_t code = keyboardBuffer[keyboardReadIndex];
    keyboardReadIndex = (keyboardReadIndex + 1) % KB_BUFFER_LENGTH;
    return (char)keyCap[code];
}

