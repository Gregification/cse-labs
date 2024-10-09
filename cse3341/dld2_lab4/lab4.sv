/**
cse3341, digital logic 2, lab 4
George Boone
1002055713
*/
module lab4 (
		input CLK,
		//input [3:0] M, Q,		// multiplicand, multiplier
		//input InM, InQ,		// load triggers
		
		// debug
		input [9:0] SWS,		// deb board switches
		input [1:0] BTN,		// dev board buttons
		input [3:0] HB_BTN,	// hexboard buttons
		output[9:0] RLEDS,	
		
		// ui
		output[3:0] HEXBOARD_CAT,
		output[6:0] HEXBOARD_SEGS
	);
	reg [7:0] Pout;
	reg [3:0] Mout, Qout;
	
	wire [31:0] _ladder;
	
	wire _clk, _reset, _done, _inm, _inq;
	wire [3:0] _m, _q;
	wire [4:0] _a;
	wire [5:0] _state;
	
	// ----------------------------------------------------------------------
	// debug & ui
	// ----------------------------------------------------------------------
	
	// inputs
	assign _clk 	= HB_BTN[3];
	assign _reset 	= SWS[8];
	assign _inm		= BTN[0];
	assign _inq		= BTN[1];
	assign _m 		= SWS[0+:4];
	assign _q		= SWS[0+:4];
	
	// outputs
	assign RLEDS[9] 		= _done;
	assign RLEDS[8] 		= _reset;
	assign RLEDS[0+:4]	= Qout;
	assign RLEDS[4+:4]	= Mout;
	
	// ----------------------------------------------------------------------
	// module
	// ----------------------------------------------------------------------
	
	SignedMultiplier_ShiftAdd_4x4 __signed_multiplier (
			.CLK(CLK),
			.RESET(_reset),
			.PLICAND(_m),
			.PLIER(_q),
			
			.DONE(_done),
			.PRODUCT(Pout),
			
			.InM(_inm),
			.InQ(_inq),
			
			.Q(Qout),
			.M(Mout),
			.A(_a),
			.STATE(_state)
		);
		
	// ui
	
	ClockLadderN #(
			.N(32)
		) __clkladder (
			.CLK(CLK),
			.CLR(1),
			.VALUE(_ladder)
		);
		
	HexBoard __hexboard(
			.LOAD(_ladder[20]),
			.RESET(_reset),
			.CLK(_ladder[16]),
			.VALUE({Mout, Qout, Pout}),
			
			.CAT(HEXBOARD_CAT),
			.SEG(HEXBOARD_SEGS)
		);
endmodule
