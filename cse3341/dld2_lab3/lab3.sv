/**
cse3341, digital logic 2, lab 3
George Boone
1002055713

	"Assume A, B, and R are eight-bit
	signed binary numbers using a two’s complement number system. CarryOut, OVR, ZERO, and
	NEG are condition codes determined by the result of the last operation performed. InA and InB
	load registers A and B, respectively. Out loads register R with the result of the last operation and
	the register CC with the last condition codes. Clear loads all zeros in all registers."
		- lab 3 : design requirements
	
*/

//size of primary registers
//		this is not actually dynamic, spaghetti code.
`define REG_P_N 8

module lab3(
		input CLEAR,
		input	OUT,
		input INA,
		input	INB,
		input CLK,
		
		output [3:0] OUTA,
		output [3:0] OUTB,
		
		//UI
		input [9:0] SWS,
		input [1:0] LP_SWS,
		
		output [3:0] HEXBOARD_CAT,
		output [6:0] HEXBOARD_SEG,
		
		output [47:0] LP_SEG,
		
		output [9:0] RLEDS
	);
	
	//wire naming scheme according to diragram on page 4 of the lab description
	wire [`REG_P_N - 1:0] A, B, Aout, Bout, R;
	wire [3:0] CCout;	//{OVR, ZERO, NEG, Cout}
	wire [2:0] CCLout;//output of condition code logic
	wire Cout;
	
	//additional wires
	wire [`REG_P_N - 1:0] NUM = SWS[0+:8];
	wire [`REG_P_N:0] CARRY_INS;
	wire ADD_SUBTRACT;
	reg OUT_DELAYED;
	wire [31:0] _ladder;
	
	assign OUT_DELAYED = !OUT; //used as a delayed call to out. this is to give things like the 
	assign A = NUM;
	assign B = NUM;
	assign ADD_SUBTRACT = SWS[9];
	
	assign RLEDS = SWS[8] ? CARRY_INS[1+:8] : CCout; 
	
	//-----------------------------------------------------------------------------------------------------
	// REGISTER MODULES
	//-----------------------------------------------------------------------------------------------------
	
	Register #(
			.SIZE(`REG_P_N)
		) __register_A (
			.TRIGGER(INA),
			.CLEAR(CLEAR),
			.VALUE_IN(A),
			.VALUE_OUT(Aout)
		);
	
	Register #(
			.SIZE(`REG_P_N)
		) __register_B (
			.TRIGGER(INB),
			.CLEAR(CLEAR),
			.VALUE_IN(B),
			.VALUE_OUT(Bout)
		);
		
	Register #(
			.SIZE(4)
		) __register_CC (
			.TRIGGER(OUT),
			.CLEAR(CLEAR),
			.VALUE_IN({CCLout, Cout}),
			.VALUE_OUT(CCout)
		);
		
	Register #(
			.SIZE(`REG_P_N)
		) __register_R (
			.TRIGGER(OUT),
			.CLEAR(CLEAR),
			.VALUE_IN(R),
			.VALUE_OUT(Rout)
		);
	
	
	//-----------------------------------------------------------------------------------------------------
	// SPECIALITY MODULES
	//-----------------------------------------------------------------------------------------------------
	
	GroupCarryLookaheadAdder __gcla (
			.A(Aout),
			.B(Bout),
			.ADD_SUBTRACT(ADD_SUBTRACT),
			
			.R(R),
			.C(CARRY_INS)
		);
	
	ConditionCodeLogic __CCL (
			.R(R),
			.CARRY_INS(CARRY_INS),
			
			.OVR(CCLout[0]),
			.ZERO(CCLout[1]),
			.NEG(CCLout[2])
		);

	//-----------------------------------------------------------------------------------------------------
	// NON CORE MODULES
	//		modeuls that arnt related to the core design. just UI
	//		note that there are some logical functions going on, this is only to get the abs of 2's comp.
	//			so the display modules will put out something legible.
	//			this is purely a post processing step, it does not effect the core modules in any way.
	//-----------------------------------------------------------------------------------------------------
	
	reg [6:0] A_abs, B_abs, R_abs; 
	
	assign A_abs = Aout[7] ? (~Aout[0+:7] + 1'd1) : Aout;
	assign B_abs = Bout[7] ? (~Bout[0+:7] + 1'd1) : Bout;
	assign R_abs = R   [7] ? (~R   [0+:7] + 1'd1) : R;
	
	ClockLadderN #(
			.N(32)
		) __clkladder (
			.CLK(CLK),
			.CLR(1),
			.VALUE(_ladder)
		);
		
	HexBoard __hexboard(
			.LOAD(OUT_DELAYED & _ladder[20]),
			.RESET(CLEAR),
			.CLK(_ladder[17]),
			.VALUE({R[7], 5'b0, R_abs}),	//shuffle sign bit over a few hex
			
			.CAT(HEXBOARD_CAT),
			.SEG(HEXBOARD_SEG)
		);
	
	// display Bout in launchpad hex slots 0, 1
	Bin2SevSegI __b2ss_h0(
		.BIN(B_abs[0+:4]),
		.INV(0),
		.SEG(LP_SEG[0+:8])
	);
	Bin2SevSegI __b2ss_h1(
		.BIN(B_abs[4+:3]), 	//exclude sign bit
		.INV(0),
		.SEG(LP_SEG[8+:8])
	);
	
	// display Bouts' sign in launchpad hex 2
	Bin2SevSegI __b2ss_h2(
		.BIN(Bout[7] ? 4'd0 : 4'd8),
		.INV(1),
		.SEG(LP_SEG[16+:8])
	);
	
	// display Aout in launchpad hex slots 3, 4
	Bin2SevSegI __b2ss_h3(
		.BIN(A_abs[0+:4]),
		.INV(0),
		.SEG(LP_SEG[24+:8])
	);
	Bin2SevSegI __b2ss_h4(
		.BIN(A_abs[4+:3]), 	//exclude sign bit
		.INV(0),
		.SEG(LP_SEG[32+:8])
	);
	
	// display Aouts' sign in launchpad hex 5
	Bin2SevSegI __b2ss_h5(
		.BIN(Aout[7] ? 4'd0 : 4'd8),
		.INV(1),
		.SEG(LP_SEG[40+:8])
	);
	
endmodule
