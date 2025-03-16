/**
cse3341, digital logic 2, lab 5
George Boone
1002055713

accepts 2 raw binary nums
*/

module RCSubtractor#(
		parameter N = 8
	)(
		input [N-1:0] A, B,
		output [N-1:0] D,
		output COUT
	);

	wire [N-1:0] _carries;
	
	assign COUT = _carries[N-1];
	
	// ripple carry design
	genvar i;
	generate
		for(i = N-1; i >= 0; i = i-1) begin : adder_array
			FullAdder _fa(
				.A(A[i]),
				.B(~B[i]),
				.CIN(i == 0 ? 1 : _carries[i-1]),
				.S(D[i]),
				.COUT(_carries[i])
			);
		end
	endgenerate
	
endmodule
