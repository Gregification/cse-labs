/**
cse3341, digital logic 2, lab 3
George Boone
1002055713

represents the module named "Condition Code Logic" on Figure 6 of design requiements.

- added input 'CARRY_OUTS' which holds the values of carry out to each bit. 
	I dont thing its possible to tell overflow otherwise only knowing the final value,
	didnt see anything in hte lab docs saying we couldnt do this either.
	
	i.e., CARRY_OUTS[0]==1 => first bit[0] had a carry out.
*/

module ConditionCodeLogic #(
		parameter N = 8
	)(
		input [N - 1:0] R,
		input [N - 1:0] CARRY_OUTS,
		
		output OVR,			//overflow
		output ZERO,		//zero
		output NEG			//negative
	);

	assign OVE	= CARRY_OUTS[N - 1] ^ CARRY_OUTS[N - 2];	// C_(n) xor C_(n-1)
	assign ZERO = R == 8'd0;	
	assign NEG	= R[N - 1];			//value of MSB
	
endmodule
