/**
	cse3341, digital logic 2, lab 7
	George Boone
	1002055713
	
	baselined off the RCSubtractor module from my lab6
*/

module ControlledIncrementor#(
		parameter N = 4
	)(
		input [N-1:0] A,
		input SELECT,
		output [N-1:0] S,
		output COUT
	);

	wire [N-1:0] _carries;
	
	assign COUT = _carries[N-1];
	
	// ripple carry design
	genvar i;
	generate
		for(i = N-1; i >= 0; i = i-1) begin : adder_array
			
			if(i != 0) begin
			
				FullAdder _fa(
					.A		(A[i]),
					.B		(0),
					.CIN	(_carries[i-1]),
					.S		(S[i]),
					.COUT	(_carries[i])
				);
				
			end else begin
			
				HalfAdder _ha(
					.A		(A[i]),
					.B		(SELECT),
					.S		(S[i]),
					.COUT	(_carries[i])
				);
				
			end
			
		end
	endgenerate
	
endmodule
