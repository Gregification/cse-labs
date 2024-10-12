/**
cse3341, digital logic 2, lab 4
George Boone
1002055713

based off of my signed multiplier, which is was based on figure 1 of the assignment
*/

`define N 4

module UnsignedMultiplier_ShiftAdd_4x4(
		input 		CLK, RESET,
		input	wire	[`N-1:0]	PLICAND, PLIER,	// multiplicand, multiplier
		
		output reg DONE,
		output reg [(`N*2)-1:0]	PRODUCT,
		
		// debug
		input InM,InQ,
		output reg [`N-1:0]	M, Q
	);	
	
	integer i;
	
	always @ (posedge CLK, posedge InQ, posedge InM, negedge RESET) begin
		if(RESET == 0) begin
			DONE = 0;
		end else
		
		if(InQ == 1) begin
			Q = PLIER;
			DONE = 0;
		end else
		if(InM == 1) begin
			M = PLICAND;
			DONE = 0;
		end else	
		
		if(DONE == 0) begin
			PRODUCT = 0;
			DONE = 1;
			
			for(i = 0; i < `N; i = i + 1) begin
				if(Q[i] == 1) begin
					PRODUCT = PRODUCT + (M << i);
				end
			end
		end
	end
	
endmodule
