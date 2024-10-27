/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713

	eight bit logical right barrel shifter
	this module only serves to wire i/o and link the barrel shifter to its test unit.
		no additional logic is provided.
*/

module lab6(
		input [9:0] DEVBOARD_SWS,
		input [1:0] DEVBOARD_BTN,
		input [3:0] KEYBOARD_BTN,
		input CLK,
		
		output[9:0] DEVBOARD_RLEDS,	
		output[47:0]DEVBOARD_SEGS,
		output[9:0] KEYBOARD_GLEDS
	);
//-----------------------------------------------------------------
// i/o
//-----------------------------------------------------------------
	wire OnOff, Clear, Set, Clock;
	wire onoff_state;
	wire [31:0] clk_ladder;
	wire [7:0] A, Y; 
	wire [2:0] shift_amount;
	
	assign OnOff 		= ~KEYBOARD_BTN[0];
	assign Clock		= clk_ladder[26];
	assign Clear 		= ~DEVBOARD_BTN[0];
	assign Set			= ~DEVBOARD_BTN[1];
	assign A				= DEVBOARD_SWS[0+:8];
	
	assign DEVBOARD_RLEDS[0+:8] 	= A;
	assign DEVBOARD_RLEDS[9] 		= onoff_state;
	assign DEVBOARD_RLEDS[8] 		= Clock;
	assign DEVBOARD_SEGS [8+:40] 	= '1;
	assign KEYBOARD_GLEDS[0+:8]	= Y;
	
//-----------------------------------------------------------------
// modules
//-----------------------------------------------------------------
	
	BarrelShifter_8r _bs8r(
			.A(A),
			.B(shift_amount),
			.Y(Y)
		);
	
	ShiftTester _st(
			.OnOff(OnOff),
			.Clear(Clear),
			.Set(Set),
			.Clock(Clock),
			
			.B(shift_amount),
			.onoff_internal(onoff_state)
		);

	Bin2SevSegI _bt7s(
			.BIN(shift_amount),
			.INV(0),
			.SEG(DEVBOARD_SEGS[0+:8])
		);

	BinCounter #(
			.N(32)
		) _bc (
			.COUNT(CLK),
			.RESET(0),
			.SET(0),
			
			.VALUE(clk_ladder)
		);
	
endmodule
