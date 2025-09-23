// Keyboard Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// 4x4 Keyboard
//   Column 0-3 outputs on PA6, PA7, PD2, PD3 are connected to cathode of diodes whose anode connects to column of keyboard
//   Rows 0-3 inputs connected to PE1, PE2, PE3, PF1 which are pulled high
//   To locate a key (r, c), the column c is driven low so the row r reads as low

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef KB_H_
#define KB_H_

#include <stdbool.h>

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initKb();
bool kbhit();
char getKey();

#endif
