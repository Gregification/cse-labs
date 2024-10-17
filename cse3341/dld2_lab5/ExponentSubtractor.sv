/**
cse3341, digital logic 2, lab 5
George Boone
1002055713

hardcoded exponent subtractor according to figure 1 of lab document
*/

module ExponentSubtractor(
		input [7:0] EA, EB,
		output Cout,
		output [7:0] ED
	);
	
	wire [7:0] d, invd;	// difference, inverted-difference
	
	RCSubtractor #(
			.N(8)
		) 	_rcs	(
			.A(EA),
			.B(EB),
			
			.D(d),
			.COUT(Cout)
		);
	
	Mux2t1 #(
			.N(8)
		)	_m2t1	(
			.CTRL(Cout),
			.A(d),
			.B(invd),
			
			.OUT(ED)
		);
		
	BintTwosComp #(
			.N(8)
		)	_bttc	(
			.BIN(d),
			
			.TWOSCOMP(invd)
		);

endmodule
