/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

`define COUNT_N 17

module dld2_lab2_partA(
		input 			ONOFF,
		input 			CLK,
		input 			RESET,
		output 	[3:0]	CAT,
		output 	[6:0]	SEG
	);
	
	logic [15:0] _count;
	
	//pretty sure I can get away with using 16bits but this less ambigious
	wire [31:0] _ladder;
	
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
			.CLK(_ladder[`COUNT_N]), 
			.CLR(RESET),
			.VALUE(_count)
		);

	MuxSevSegController _muxedSevSegController(
			.LOAD(_ladder[`COUNT_N]), 
			.MUX_CLK(_ladder[17]),
			.RESET(RESET),
			.HEX(_count),
			.CAT(CAT),
			.SEG(SEG)
		);	
	
endmodule 