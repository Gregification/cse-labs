/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
	
	2:1 multiplexer. 
	low signal selects A
	
	- bsaelined from my lab6
*/

module Mux2t1#(
		parameter N = 1
	)(
		input 				SELECTOR,
		input [N-1:0]		A, B,
		
		output[N-1:0]		SELECTED
	);

	assign SELECTED = SELECTOR ? B : A;

endmodule