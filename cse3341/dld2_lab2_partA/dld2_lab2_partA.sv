/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/


module dld2_lab2_partA(
		input 			ONOFF,
		input 			CLK,
		input 			RESET,
		output	[9:0] RLED,
		output 	[3:0]	CAT,
		output 	[6:0]	SEG
	);
	
	logic [15:0] _count;
	
	//pretty sure I can get away with using 16bits but this less ambigious
	wire [31:0] _ladder;
	
	assign RLED[9] = ONOFF;
	assign RLED[8] = RESET;
	assign RLED[7] = _ladder[23];
	assign RLED[0+:4] = CAT;
	
	ClockLadderN #(
			.N(32)
		) __clock_ladder (
			.CLK(CLK & ONOFF), //functionality for toggling the clock
			.CLR(RESET),
			.VALUE(_ladder)
		);
		
	//clock ladder used as the binary counter
	ClockLadderN #(
			.N(16)
		) __binary_counter (
			.CLK(_ladder[22]), 
			.CLR(RESET),
			.VALUE(_count)
		);

	MuxSevSegController _muxedSevSegController(
			.LOAD(_ladder[22]), 
			.MUX_CLK(_ladder[17]),
			.RESET(RESET),
			.HEX(_count),
			.CAT(CAT),
			.SEG(SEG)
		);	
	
endmodule 