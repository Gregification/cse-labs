// (TOP LEVEL)
// Arithmetic Unit Top Level

//////////////////////////////////////////////////////
//
// MODIFY THIS FILE
// Goal 1: Instantiate the proper modules as listed below
// Goal 2: Once instantiated, connect the inputs, wires, and outputs
// Goal 3: Compile, then add pin assignments
//
//////////////////////////////////////////////////////

module labNine(
		input [7:0] X,											// (DO NOT MODIFY LINE)
		input InA, InB, Out, Clear, Add_Subtract,		// (DO NOT MODIFY LINE)
		output [7:0] Result,									// (DO NOT MODIFY LINE)
		output [3:0] Flags,									// (DO NOT MODIFY LINE)
		output [0:6] HEX2, HEX1, HEX0						// (DO NOT MODIFY LINE)
	);

	wire [7:0] Aout, Bout, Rout;							// (DO NOT MODIFY LINE)
	wire [3:0] Fout;											// (DO NOT MODIFY LINE)

// * Instantiate 8-bit NBitRegister (RegisterA) using X *
	NBitRegister#(4'd8) RegisterA (
		.D(X)			,
		.CLK(InA)	,
		.CLR(Clear)	,
		.Q(Aout)
	);

// * Instantiate 8-bit NBitRegister (RegisterB) using X *
	NBitRegister#(4'd8) RegisterB (
		.D(X)			,
		.CLK(InB)	,
		.CLR(Clear)	,
		.Q(Bout)
	);

// * Instantiate 8-bit RippleCarryAdderStructural (RCA) *
	RippleCarryAdderStructural RCA(
		.A(Aout)			,
		.B(Bout)			,
		.Add_Subtract(Add_Subtract)	,
		.S(Result)		,
		.Flags(Flags)	,
	);

// * Instantiate 4-bit NBitRegister (RegisterCC) using Flags *
	NBitRegister#(3'd4) RegisterCC (
		.D(Flags)	,
		.CLK(Out)	,
		.CLR(Clear)	,
		.Q(Fout)
	);

// * Instantiate 8-bit NBitRegister (RegisterR) using Result *
	NBitRegister#(4'd8) RegisterR (
		.D(Result)	,
		.CLK(Out)	,
		.CLR(Clear)	,
		.Q(Rout)
	);

// * Instantiate 4-bit to 1-hex bin2sev using Rout[3:0] *
	binary2seven bin2sev1H (
		.BIN(Rout[7:4])	,
		.SEV(HEX1)
	);
	
// * Instantiate 4-bit to 1-hex bin2sev using Rout[7:4] *
	binary2seven bin2sev0H (
		.BIN(Rout[3:0])	,
		.SEV(HEX0)
	);
	
// * Instantiate 4-bit to 1-hex bin2sev using Fout *
	binary2seven bin2sev2H (
		.BIN(Fout)	,
		.SEV(HEX2)
	);

endmodule
