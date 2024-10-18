/**
cse3341, digital logic 2, lab 5
George Boone
1002055713
*/

module BintTwosComp#(
		parameter N = 8
	)(
		input [N-1:0] BIN,
		output reg [N-1:0] TWOSCOMP,
		
		// debug
		output reg COUT
	);

	wire [N-1:0] _carries;
	
	assign COUT = ~_carries[N-1];
	
	// ripple carry design
	genvar i;
	generate
		for(i = N-1; i >= 0; i = i-1) begin : adder_array
			FullAdder _fa(
				.A(~BIN[i]),
				.B(0),
				.CIN(i == 0 ? 1 : _carries[i-1]),
				.S(TWOSCOMP[i]),
				.COUT(_carries[i])
			);
		end
	endgenerate
	
endmodule
