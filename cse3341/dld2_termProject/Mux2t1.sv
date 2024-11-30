/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713
	
	2:1 multiplexer. 
	low signal selects A
*/

module Mux2t1(
		input SELECTOR,
		input A, B,
		output SELECTED
	);

	assign SELECTED = SELECTOR ? B : A;

endmodule