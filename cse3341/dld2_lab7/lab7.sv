/**
	cse3341, digital logic 2, lab 7
	George Boone
	1002055713
*/

module lab7(
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
	wire [7:0] A, A_incremented, A_decremented;
	wire Select;
	wire ShowIncElseDec;
	
	// i
	assign A 					= DEVBOARD_SWS[0+:8];
	assign Select				= DEVBOARD_SWS[8];
	assign ShowIncElseDec	= DEVBOARD_SWS[9];
	
	// o
	assign DEVBOARD_RLEDS[0+:8]	= ShowIncElseDec ? A_incremented : A_decremented;
	assign DEVBOARD_RLEDS[9] 		= ShowIncElseDec;
	
	
//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
	ControlledIncrementor#(
		.N(8)
	) _ci (
		.A(A),
		.SELECT(Select),
		.S(A_incremented)
	);
	
	ControlledDecrementer#(
		.N(8)
	) _cd (
		.A(A),
		.SELECT(Select),
		.S(A_decremented)
	);
	
endmodule
