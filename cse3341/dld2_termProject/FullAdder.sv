/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
	
	a generic full adder
*/

module FullerAdder(
		input 	A,
		input 	B,
		input		C,
		
		output 	R,
		output	COUT
	);
	
	wire axb, ac, axbc;
	
	xor(axb, 	A, 	B);
	and(ab,		A,		B);
	xor(R,		axb,	C);
	and(axbc,	axb,	C);
	or	(COUT,	axbc,	ab);
	
endmodule
