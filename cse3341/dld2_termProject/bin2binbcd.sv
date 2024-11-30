/**
	cse3341, digital logic 2, lab 8
	George Boone
	1002055713
*/

module bin2binbcd 
	#(parameter                 WIDTH  = 32              , // Bit Width
	  parameter                 DIGITS = 10              ) // Digits
	 (input         [WIDTH-1:0] bin                      , // 2's Complement Binary Input
	  output 						 sign							  ,
	  output    reg [3:0]       bcd         [DIGITS-1:0]); // BCD Output
	
	integer i,j;
	
	always @ (bin) begin
		for(i = 0; i < DIGITS; i = i + 1) begin : match_arr
			if((bin & (1 << i)) != 0)
				bcd[i]	= 4'd1;
			else
				bcd[i]	= 4'd0;
		end
	end

endmodule
