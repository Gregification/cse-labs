module OU(
		input [7:0] TC_in, //2's compliment
		output [0:6] SEVSEG_ONES,
		output [0:6] SEVSEG_TENS,
		output [0:6] SEVSEG_HUNDREDS,
		output [0:6] SEVSEG_SIGN
	);
	
	wire [7:0] SM_in;
	wire [3:0] BCD_ONES;
	wire [3:0] BCD_TENS;
	wire [1:0] BCD_HUNDREDS;
	
	assign SEVSEG_SIGN = TC_in[7] ? 7'b1111110 : 7'b1111111;
	
	twoSIGN _tc2bsm (
		.A(TC_in),
		.B(SM_in)
	);
	
	binary2bcd _b2bcd (
		.A(SM_in),
		.ONES(BCD_ONES),
		.TENS(BCD_TENS),
		.HUNDREDS(BCD_HUNDREDS)
	);
	
	binary2seven _bin2seg_ones (
		.BIN(BCD_ONES),
		.SEV(SEVSEG_ONES)
	);
	
	binary2seven _bin2seg_tens (
		.BIN(BCD_TENS),
		.SEV(SEVSEG_TENS)
	);
	
	binary2seven _bin2seg_hundreds (
		.BIN(BCD_HUNDREDS),
		.SEV(SEVSEG_HUNDREDS)
	);

endmodule