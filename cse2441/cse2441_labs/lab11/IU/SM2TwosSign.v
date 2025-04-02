module SM2TwoSign(
			input [7:0] SM,
			output reg [7:0] twosComp
	);
	
	always twosComp = (SM[7] == 1) ? (~SM + 1'b1) : SM;
	
endmodule