/**
cse3341, digital logic 2, lab 5
George Boone
1002055713

hardcoded exponent subtractor according to figure 1 of lab document
*/

module ExponentSubtractor#(
		parameter N = 8
	)(
		input [N-1:0] EA, EB,
		output Cout,
		output [N-1:0] ED
	);
	
	wire [N-1:0] d, invd;	// difference, inverted-difference
	
	RCSubtractor #(
			.N(N)
		) 	_rcs	(
			.A(EA),
			.B(EB),
			
			.D(d),
			.COUT(Cout)
		);
	
	Mux2t1 #(
			.N(N)
		)	_m2t1	(
			.CTRL(Cout),
			.A(invd),
			.B(d),
			
			.OUT(ED)
		);
		
	BintTwosComp #(
			.N(N)
		)	_bttc	(
			.BIN(d),
			
			.TWOSCOMP(invd)
		);

endmodule
