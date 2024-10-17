/**
cse3341, digital logic 2, lab 5
George Boone
1002055713
*/

module BintTwosComp#(
		parameter N = 8
	)(
		input [N-1:0] BIN,
		output [N-1:0] TWOSCOMP
	);

	wire [N-1:0] _carries;
	
	// ripple carry design
	genvar i;
	generate
		for(i = 0; i < N; i = i+1) begin : adder_array
			FullAdder _fa(
				.A(~B[i]),
				.B(0),
				.CIN(i == 0 ? 1 : _carries[i-1]),
				.S(TWOSCOMP[i]),
				.COUT(_carries[i])
			);
		end
	endgenerate
	
endmodule
