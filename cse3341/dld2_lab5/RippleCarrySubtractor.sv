/**
cse3341, digital logic 2, lab 5
George Boone
1002055713

accepts 2 raw binary nums
*/

module RCSubtractor#(
		parameter N = 4
	)(
		input [3:0] A, B,
		output [3:0] D,
		output COUT
	);

	wire [3:0] _carries, _bactual;
	
	assign COUT 		= _carries[3];	// carries are really just borrows
	assign _bactual 	= ~B; 			// equilvilent to "B[n] ^ 1"
	
	// ripple carry design
	genvar i;
	generate
		for(i = 0; i < N; i = i+1) begin : adder_array
			FullAdder _fa(
				.A(A[i]),
				.B(_bactual[i]),
				.CIN(i == 0 ? 1 : _carries[i-1]),
				.S(D[i]),
				.COUT(_carries[i])
			);
		end
	endgenerate
	
endmodule
