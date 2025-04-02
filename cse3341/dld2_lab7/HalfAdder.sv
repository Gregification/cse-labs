/**
	cse3341, digital logic 2, lab 7
	George Boone
	1002055713
*/

module HalfAdder(
		input A, B,
		output S, COUT
	);
	
	assign S 	= A ^ B;
	assign COUT = A & B;
	
endmodule
