/**
cse3341, digital logic 2, lab 5
George Boone
1002055713
*/

module Mux2t1#(
		parameter N = 8
	)(
		input CTRL,				// 0:A, 1:B
		input [N-1:0] A, B,
		output wire [N-1:0] OUT
	);
	
	always @ (edge CTRL) 
		if(CTRL == 0)
			OUT = A;
		else
			OUT = B;
	
endmodule
