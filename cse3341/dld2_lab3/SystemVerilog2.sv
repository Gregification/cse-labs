/**
cse3341, digital logic 2, lab 3
George Boone
1002055713
*/

module FullAdder(
	input A,	//a bit
	input B,	//a bit
	input C, //carry in
	
	output Cout,	//carry out
	output G,		//generate
	output P			//propagate
);
	
	assign G = A & B;

endmodule

