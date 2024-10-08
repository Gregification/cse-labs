/**
cse3341, digital logic 2, lab 4
George Boone
1002055713

hardcoded 4 bit cla adder
*/

module CLAdder_4(
		input [3:0] A, B,
		input CIN,
		output [3:0] R,
		output COUT
	);
	
	wire [3:0] g, p;
	wire [4:0] c;
	
	assign c[0] = CIN;
	assign COUT = c[4];
	
	assign c[1] = (g[0]) | (c[0] & p[0]);
	assign c[2] = (g[0] & p[1]) | (c[0] & p[0] & p[1]) | (g[1]);
	assign c[3] = (g[0] & p[1] & p[2]) | (c[0] & p[0] & p[1] & p[2]) | (g[1] & p[2]) | (g[2]);
	assign c[4] = (g[0] & p[1] & p[2] & p[3]) | (c[0] & p[0] & p[1] & p[2] & p[3]) | (g[1] & p[2] & p[3]) | (g[2] & p[3]) | (g[3]);
	
	assign g[0] = a[0] & b[0];
	assign g[1] = a[1] & b[1];
	assign g[2] = a[2] & b[2];
	assign g[3] = a[3] & b[3];

	assign p[0] = a[0] ^ b[0];
	assign p[1] = a[1] ^ b[1];
	assign p[2] = a[2] ^ b[2];
	assign p[3] = a[3] ^ b[3];

	assign R[0] = p[0] ^ c[0];
	assign R[1] = p[1] ^ c[1];
	assign R[2] = p[2] ^ c[2];
	assign R[3] = p[3] ^ c[3];

endmodule
