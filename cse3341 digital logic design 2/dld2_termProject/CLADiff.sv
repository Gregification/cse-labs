/**
cse3341, digital logic 2, term project
George Boone
1002055713
*/

module CLADiff#(
		parameter N = 8
	)(
		input [N-1:0] 		A, B,
		
		output [N-1:0]		R,
		output wire			COUT	
	);
	
	wire 	[N-1:0]		raw, neg_raw;
	
	assign R = COUT ? raw : neg_raw;

	
	CLA#(
		.N(N)
	) _subtractor (
		.A(A),
		.B(B),
		.ADD_SUB(1),		// 0:add, 1:sub
		
		.R(raw),
		.COUT(COUT)
	);
	
	CLA#(
		.N(N)
	) _flipper (
		.A(1),
		.B(~raw),
		.ADD_SUB(0),		// 0:add, 1:sub
		
		.R(neg_raw)
	);
	
endmodule
