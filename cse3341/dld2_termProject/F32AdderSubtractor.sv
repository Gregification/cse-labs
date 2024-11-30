/**
cse3341, digital logic 2, term project
George Boone
1002055713

float 32 adder & subtractor

- most of this would maybe work for any size float
*/

module F32AdderSubtractor(
		input [31:0]		A, B,
		input					SUBTRACT,
		
		output[31:0]		R,
		output				UNDERFLOW,
		output				OVERFLOW
	);

	//---------------------------internal----------------------------
	
	wire 	[22:0] 	am, bm;	// mantissa
	wire	[7:0]		ae, be;	// exponent
	wire				as, bs;	// sign
	
	assign am = A[0+:23];
	assign ae = A[23+:8];
	assign as = A[31];
	assign bm = B[0+:23];
	assign be = B[23+:8];
	assign bs = B[31];
	
	//------------------inner module connections---------------------
	
	wire	[23:0]	m_shift, m_add;
	wire	[7:0]		e_inc;
	wire	[4:0]		e_diff;
	wire				e_diff_cout;
	wire				cla_cout;	
	
	//---------------------------modules-----------------------------
	
	BarrelRightShifter#(
			.N(31),
			.NLayers($clog2(23))
		) mantissa_shifter (
//			.IN(32'h0000_0F00),
//			.CTRL(0),
			
//			.OUT()
		);
	
endmodule
