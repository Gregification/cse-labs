/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713
*/

module BinCounter #(
		parameter N = 8
	)(
		input COUNT, RESET, SET,
		
		output reg [N-1:0] VALUE
	);
	
	always_ff @ (posedge COUNT, posedge RESET, posedge SET) begin
		if(RESET == 1) begin
			VALUE = 0;
		end else if(SET == 1) begin
			VALUE = '1;
		end else if(COUNT == 1) begin
			VALUE = VALUE + 1;
		end
	end
	
endmodule
