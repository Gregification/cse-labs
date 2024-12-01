/**
cse3341, digital logic 2, term project
George Boone
1002055713

float 32 adder & subtractor

- most of this would maybe work for any size float
*/

module F32AdderSubtractor(
		input [31:0]		A, B,
		input					OP,			// 0:add, 1:sub
		
		output[31:0]		R,
		output				UNDERFLOW,
		output				OVERFLOW,
		
		input	[7:0]			debug
	);

	//---------------------------internal----------------------------
	
	wire 	[22:0] 	ma, mb;	// mantissa
	wire	[7:0]		ea, eb;	// exponent
	wire				sa, sb;	// sign
	
	assign ma = A[0+:23];
	assign ea = A[23+:8];
	assign sa = A[31];
	assign mb = B[0+:23];
	assign eb = B[23+:8];
	assign sb = B[31];
	
	//------------------inner module connections---------------------
	
	wire	[23:0]	m_shift_mux, m_shift, m_add, cla_res, m_norm;
	wire	[7:0]		e_mux, e_ctrled;
	wire	[7:0]		e_diff;
	wire				e_diff_cout;
	wire				cla_cout;
	wire				e_ctrled_cout;
	
	always case(debug)
			default	: begin
				R[23+:8]	= e_ctrled;
				R[0+:23]	= m_norm;
				R[31]		= e_ctrled_cout;
			end
			8'h01 	: R = 1;
			8'h02 	: R = e_ctrled_cout;
			8'h04 	: R = e_diff_cout;
			8'h08 	: R = 4;
			8'h10	 	: R = e_mux;
			8'h20 	: R = e_ctrled;
			8'h40 	: R = ma;
			8'h80 	: R = m_shift;
		endcase
	
	//---------------------------modules-----------------------------
	
	CLADiff#(
			.N(8)
		) _exponent_subtractor (
			.A(ea),
			.B(eb),
			
			.R(e_diff),
			.COUT(e_diff_cout)
		);
	
	Mux2t1#(
		.N(23)
	) _mantissa_shifter_mux (
		.SELECTOR(e_diff_cout),
		.A(ma),
		.B(mb),
		
		.SELECTED(m_shift_mux[0+:23])
	);
	assign m_shift_mux[23] = 1;
	
	BarrelShifter#(
			.N(24),
			.NLayers(5),
			.SHIFT_LEFT(0)
		) mantissa_shifter (
			.IN(m_shift_mux),
			.CTRL(e_diff),
			
			.OUT(m_shift)
		);
	
	BarrelShifter#(
			.N(24),
			.NLayers(1),
			.SHIFT_LEFT(0)
		) _mantissa_normalizer (
			.IN(cla_res),
			.CTRL(cla_cout),
			
			.OUT(m_norm)
		);
	
	Mux2t1#(
		.N(8)
	) _expo_mux (
		.SELECTOR(e_diff_cout),
		.A(eb),
		.B(ea),
		
		.SELECTED(e_mux)
	);
	
	CLA#(
		.N(24)
	) _ctrled_expo (
		.A(e_mux),
		.B(cla_cout),
		.ADD_SUB(OP),		// 0:add, 1:sub
		
		.R(e_ctrled),
		.COUT(e_ctrled_cout)
	);
	
	Mux2t1#(
		.N(23)
	) _the_other_mantissa_mux (
		.SELECTOR(e_diff_cout),
		.A(mb),
		.B(ma),
		
		.SELECTED(m_add)
	);
	
	CLA#(
		.N(24)
	) _cla_unit (
		.A(m_shift),
		.B(e_add),
		.ADD_SUB(OP),		// 0:add, 1:sub
		
		.R(cla_res),
		.COUT(cout)
	);
		
endmodule
