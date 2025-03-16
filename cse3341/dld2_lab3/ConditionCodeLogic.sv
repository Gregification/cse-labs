/**
cse3341, digital logic 2, lab 3
George Boone
1002055713

represents the module named "Condition Code Logic" on Figure 6 of design requiements.

- added input 'CARRY_OUTS' which holds the values of carry out to each bit. 
	I dont thing its possible to tell overflow otherwise only knowing the final value,
	didnt see anything in hte lab docs saying we couldnt do this either.
	
	i.e., CARRY_OUTS[0]==1 => first bit[0] had a carry out.
	
- hardcoded values because of spaghetti code
*/

module ConditionCodeLogic (
		input [7:0] R,
		input [8:0] CARRY_INS,
		
		output OVR,			//overflow
		output ZERO,		//zero
		output NEG			//negative
	);
	
	assign OVR	= CARRY_INS[8] ^ CARRY_INS[7];	// C_(n) xor C_(n-1)
	assign ZERO = R == 0;	
	assign NEG	= R[7];			//value of MSB
	
endmodule
