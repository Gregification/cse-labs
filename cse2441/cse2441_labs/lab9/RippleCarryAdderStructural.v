// (NOT TOP LEVEL)
// Ripple Carry Adder Structural Model
// This module should be instantiated once in the top level

//////////////////////////////////////////////////////
//
// MODIFY THIS FILE
// Goal 1: Extend Ripple Carry Adder from 4 bits to 8 bits
// Goal 2: Implement the conditional flag code logic
//
//////////////////////////////////////////////////////

module RippleCarryAdderStructural // Name of Module
	(
		input [7:0] A, B,				// * Modify input vector size from 4->8 *
		input Add_Subtract,			// DO NOT MODIFY
		output [7:0] S,				// * Modify output vector size from 4->8 *
		output [3:0] Flags			// DO NOT MODIFY
	);

	wire [8:0] C; 						// * Modify wire vector size from 5->9 *
	wire cout, ovr, neg, zero;		// DO NOT MODIFY
	assign C[0] = Add_Subtract;	// DO NOT MODIFY
	assign Flags = {cout, ovr, neg, zero}; // DO NOT MODIFY

	FAbehav s0 (A[0], B[0]^C[0], C[0], S[0], C[1]); // DO NOT MODIFY
	FAbehav s1 (A[1], B[1]^C[0], C[1], S[1], C[2]); // DO NOT MODIFY
	FAbehav s2 (A[2], B[2]^C[0], C[2], S[2], C[3]); // DO NOT MODIFY
	FAbehav s3 (A[3], B[3]^C[0], C[3], S[3], C[4]); // DO NOT MODIFY
	FAbehav s4 (A[4], B[4]^C[0], C[4], S[4], C[5]);
	FAbehav s5 (A[5], B[5]^C[0], C[5], S[5], C[6]);
	FAbehav s6 (A[6], B[6]^C[0], C[6], S[6], C[7]);
	FAbehav s7 (A[7], B[7]^C[0], C[7], S[7], C[8]);

	assign cout = C[8]; 		// * Modify output flag and implement cout logic *
	assign ovr = C[8] ^ C[7];		// * Modify output flag to implement ovr logic *
	assign neg = S[7];		// * Modify output flag to implement neg logic *
	assign zero = S == 1'b0;		// * Modify output flag to implement zero logic *

endmodule
