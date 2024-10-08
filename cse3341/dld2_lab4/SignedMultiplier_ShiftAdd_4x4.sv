/**
cse3341, digital logic 2, lab 4
George Boone
1002055713

based off of figure 1 of the assignment

	- hardcoded 4 bits each input, much change control counter logic if N bits changed
*/

`define N 4

module SignedMultiplier_ShiftAdd_4x4(
		input 		CLK,
		input 		RESET,
		input	[`N-1:0]	PLICANT, PLIER,	// multiplicand, multiplier
		
		output wire	HALT
		output wire [`N*2-1:0]	PRODUCT
	);
	
	// shift signal
	wire _shift_clk;
	
	
	
	// 2-bit counter logic
	logic [1:0] _2b_counter;
	wire _counter_not_zero;
	assign _counter_not_zero = _2b_counter[0] & _2b_counter[1];
	
	// register A
	Register #(
			.N(N)
		) __register_A (
			.TRIGGER(_shift_clk),
			.CLEAR(RESET),
			.VALUE_IN()
		);
	
	
endmodule
