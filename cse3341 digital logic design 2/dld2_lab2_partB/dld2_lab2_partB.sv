/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

`define HEXBOARD_MUX_CLKLADDER_B 17
`define KEYPAD_CLKLADDER_B 17

module dld2_lab2_partB(
		input 		CLK,
		input			RESET,
		input [3:0] KEYPAD_ROWS,
		
		output reg [9:0]	RLEDS,
		output[3:0] KEYPAD_COLS,
		output[3:0] HEXBOARD_CAT,
		output[6:0] HEXBOARD_SEG
	);
	
	//clock ladder
	wire [31:0] _ladder;
	
	//keypad entrie
	wire [15:0] _value;
	
	//if a key is pressed
	wire _pressed;
	
	wire [3:0] _val;
	assign RLEDS[9]	= _pressed;
	assign RLEDS[0+:4]= _val;
	
	always_ff @ (posedge _pressed)
		RLEDS[4+:4] = _val;
	
	//set clock ladder
	ClockLadderN #(
			.N(32)
		) __clock_ladder (
			.CLK(CLK),
			.CLR(RESET),
			.VALUE(_ladder)
		);
		
	//hex board display controller
	MuxSevSegController _muxedSevSegController(
		.LOAD(_pressed & _ladder[20]), //must be tied to ladder. otherwise KeyPad register shift happens after this has already loaded => loads the old value, not the new one like it should.
		.MUX_CLK(_ladder[`HEXBOARD_MUX_CLKLADDER_B]),
		.RESET(RESET),
		
		.HEX(_value),
		.CAT(HEXBOARD_CAT),
		.SEG(HEXBOARD_SEG)
	);
	
	KeyPad #(
			.N(4)
		) __key_pad (
			.CLK(_ladder[`KEYPAD_CLKLADDER_B]),
			.SHIFT(_pressed),
			.RESET(RESET),
			
			.ROWS(KEYPAD_ROWS),
			
			.COLS(KEYPAD_COLS),
			.PRESSED(_pressed),
			.NXTVAL(_val),
			.VALUE(_value)
		);
	
endmodule