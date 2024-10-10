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
		output[47:0] SEGS,
		
		// ui
		output[3:0] HEXBOARD_CAT,
		output[6:0] HEXBOARD_SEGS
	);
	reg [7:0] Pout;
	reg [3:0] Mout, Qout, _a;
	
	wire [31:0] _ladder;
	
	wire _clk, _reset, _done, _inm, _inq, _q_sout;
	wire [3:0] _input;
	wire [5:0] _state;
	
	// ----------------------------------------------------------------------
	// debug & ui
	// ----------------------------------------------------------------------
	
	// inputs
	assign _clk 	= ~HB_BTN[3];	// btns are inverted to act as pull low
	assign _reset 	= HB_BTN[2];
	assign _inm		= ~BTN[0];	
	assign _inq		= ~BTN[1];
	assign _input	= SWS[0+:4];
	
	// outputs
	assign RLEDS[8] 		= _q_sout;
	
	// ----------------------------------------------------------------------
	// modules
	// ----------------------------------------------------------------------
	
	SignedMultiplier_ShiftAdd_4x4 __signed_multiplier (
			.CLK(_clk),
			.RESET(_reset),
			.PLICAND(_input),
			.PLIER(_input),
			
			.DONE(_done),
			.PRODUCT(Pout),
			
			.InM(_inm),
			.InQ(_inq),
			
			.Q(Qout),
			.M(Mout),
			.A(_a),
			.STATE(_state),
			._Q_shift_out(_q_sout)
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
			.LOAD(_ladder[21]),
			.RESET(_reset),
			.CLK(_ladder[18]),
			.VALUE({Mout, Qout, Pout}),
			
			.CAT(HEXBOARD_CAT),
			.SEG(HEXBOARD_SEGS)
		);
		
	// ----------------------------------------------------------------------
	// additional debugging
	// ----------------------------------------------------------------------
	
	// this is here to dim the hex output, im doing this lab in a dim room and the hex lights are insufferably bright
	wire [47:0] _segs;
	assign SEGS = _ladder[20] ? _segs : ~(48'b0);
	
	// display controller state on launchpad hex slots 0, 1
	Bin2SevSegI __b2ss_h0(
		.BIN(_state[0+:4]),
		.INV(0),
		.SEG(_segs[0+:8])
	);
	Bin2SevSegI __b2ss_h1(
		.BIN(_state[4+:2]),
		.INV(0),
		.SEG(_segs[8+:8])
	);
	
	// display Q shift out on launchpad hex 2
	Bin2SevSegI __b2ss_h2(
		.BIN(_q_sout),
		.INV(0),
		.SEG(_segs[16+:8])
	);
	
	// display Q[0] on launchpad hex 3
	Bin2SevSegI __b2ss_h3(
		.BIN(Qout[0]),
		.INV(0),
		.SEG(_segs[24+:8])
	);
	
	// display A on launchpad hex 4
	Bin2SevSegI __b2ss_h4(
		.BIN(_a[0+:4]),
		.INV(0),
		.SEG(_segs[32+:8])
	);
	
	// not used
	Bin2SevSegI __b2ss_h5(
		.BIN(4'd8),
		.INV(1),
		.SEG(_segs[40+:8])
	);
	
endmodule
