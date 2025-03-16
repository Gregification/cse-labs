/**
	cse3341, digital logic 2, lab 7
	George Boone
	1002055713
*/

module FullAdder(
		input A, B, CIN,
		output S, COUT
	);
	
	wire axb;
	
	assign axb 	= A ^ B;
	assign S 	= axb ^ CIN;
	assign COUT = (A & B) | (axb & CIN);
	
endmodule
