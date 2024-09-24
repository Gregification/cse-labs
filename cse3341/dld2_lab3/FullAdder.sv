/**
cse3341, digital logic 2, lab 3
George Boone
1002055713
*/

module FullAdder(
	input A,		//a bit
	input B,		//a bit
	input Cin,	//carry in
	
	output S			//sum
	output Cout,	//carry out
	output G,		//generate
	output P			//propagate, xor
);
	wire AxB = A ^ B;

	assign G = A & B;
	assign P = AxB;
	assign S = AxB ^ Cin;
	assign Cout = (A & B) | (AxB & Cin);
	
endmodule

