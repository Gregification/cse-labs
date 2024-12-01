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
	
	wire	[23:0]	m_shift_mux, m_shift, m_add, cla_res;
	wire 	[23:0]	m_norm, m_norm_left_shifted, m_norm_right_shifted;
	wire	[7:0]		e_mux, e_ctrled;
	wire	[7:0]		e_diff;
	wire				e_diff_cout;
	wire				cla_cout;
	wire				e_ctrled_cout;
	
	assign m_norm = OP ? m_norm_left_shifted : m_norm_right_shifted;
	
	always case(debug)
			default	: begin
				// packed assignments get reversed
				R[23+:8]	= e_ctrled;
				R[0+:23]	= m_norm;
				R[31]		= !e_diff_cout;
			end
			8'h01 	: R = e_mux;
			8'h02 	: R = e_ctrled;
			8'h03 	: R = e_ctrled_cout;
			8'h04 	: R = 4;
			8'h05	 	: R = e_diff;
			8'h06 	: R = e_diff_cout;
			8'h07 	: R = m_shift_mux;
			8'h08 	: R = m_shift;
			8'h09 	: R = 9;
			8'h0A 	: R = m_add;
			8'h0B 	: R = m_norm_left_shifted;
			8'h0C 	: R = cla_res;
			8'h0D 	: R = 13;
			8'h0E 	: R = cla_cout;
			8'h0F 	: R = OP;
			8'h10 	: R = m_norm_right_shifted;
			8'h11 	: R = 17;
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
			.NLayers(6),
			.SHIFT_LEFT(0)
		) mantissa_shifter (
			.IN(m_shift_mux),
			.CTRL(e_diff),
			
			.OUT(m_shift)
		);
	
	BarrelShifter#(
			.N(24),
			.NLayers($clog2(24)),
			.SHIFT_LEFT(0)
		) _mantissa_normalizer_right (
			.IN(cla_res),
			.CTRL(cla_cout),
			
			.OUT(m_norm_right_shifted)
		);
		
	BarrelShifter#(
			.N(24),
			.NLayers($clog2(24)),
			.SHIFT_LEFT(1)
		) _mantissa_normalizer_left (
			.IN(cla_res),
			.CTRL(cla_cout),
			
			.OUT(m_norm_left_shifted)
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
		
		.SELECTED(m_add[0+:23])
	);
	assign m_add[23] = 1;
	
	CLA#(
		.N(24)
	) _cla_unit (
		.A(m_add),
		.B(m_shift),
		.ADD_SUB(OP),		// 0:add, 1:sub
		
		.R(cla_res),
		.COUT(cla_cout)
	);
endmodule
