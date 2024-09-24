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
`define REG_P_N 8

module lab3(
		input [`REG_P_N - 1:0] A,
		input [`REG_P_N - 1:0] B,
		input ADD_SUBTRACT,
		input CLEAR,
		input	OUT,
		input INA,
		input	INB, 
		
		output [3:0] OUTA,
		output [3:0] OUTB
		
		//UI
		output [3:0] HEXBOARD_CAT,
		output [6:0] HEXBOARD_SEG
	);
	
	//wire naming scheme according to diragram on page 4 of the lab description
	wire [`REG_P_N - 1:0] Aout;
	wire [`REG_P_N - 1:0] Bout;
	wire [`REG_P_N - 1:0] R;
	wire [3:0] CCout;
	wire [2:0] CCLout;//output of condition code logic
	wire Cout;
	
	//additional wires
	wire [`REG_P_N - 1:0] CARRY_OUTS;
	reg OUT_DELAYED;
	
	assign OUT_DELAYED = !OUT; //used as a delayed call to out. this is to give things like the 
	
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
	
	ConditionCodeLogic #(
			.N(`REG_P_N)
		) __CCL (
			.R(R),
			.CARRY_OUTS(CARRY_OUTS),
			
			.OVR(CCLout[0]),
			.ZERO(CCLout[1]),
			.NEG(CCLout[2])
		);

	//-----------------------------------------------------------------------------------------------------
	// NON CORE MODULES
	//		modeuls that arnt related to the core design. mostly just UI
	//-----------------------------------------------------------------------------------------------------
	
	HexBoard __hexboard(
			.LOAD(OUT),
			.RESET(CLEAR),
			.CLK(OUT_DELAYED),
			.VALUE(R),
			
			.CAT(HEXBOARD_CAT),
			.SEG(HEXBOARD_SEG)
		);
		
endmodule
