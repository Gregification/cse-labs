/**
cse3341, digital logic 2, lab 4
George Boone
1002055713
*/

module Subtractor#(
		parameter N = 4
	)(
		input [N-1:0] A, B,
		
		output [N:0] S
	);
	
	assign S = A - B;
	
endmodule
