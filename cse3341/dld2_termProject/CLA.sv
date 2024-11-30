/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
	
	- code baselined from class slides 7.1, slide#50
*/

module CLA#(
		parameter N = 32
	) (
		input	[N-1:0] 	A,
		input	[N-1:0] 	B,
		input				ADD_SUB,		// 0:add, 1:sub
		
		output[N-1:0] 	R,
		output			COUT
	);

	wire	[N:0]		C;
	wire	[N-1:0]	G, P, S;
	
	assign C[0] 	= ADD_SUB;
	assign COUT		= C[N];
	assign R 		= S;
	
	genvar i;
		for(i = 0; i < N; i +=1) begin : full_adders
			FullerAdder FullAdder_inst (
				.A(A[i]),
				.B(B[i] ^ ADD_SUB),
				.C(C[i]),
				
				.R(S[i])
			);
		end
	endgenerate
	
	genvar j;
	generate
		for(j = 0; i < N; j += 1) begin : term_gen
			assign G[j]		= A[j] & B[j];
			assign P[j]		= A[j] | B[j];
			assign C[j+1]	= G[j] | (P[j] & C[j]);
		end
	endgenerate
	
endmodule
