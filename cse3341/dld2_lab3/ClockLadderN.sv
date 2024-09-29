//N-bit clock ladder
module ClockLadderN #( parameter N = 4 ) (
		input CLK,	//clock
		input CLR,	//clear/reset (sets [VALUE] to zero)
		output logic [N-1:0] VALUE
	);
	
	always_ff @ (posedge CLK, negedge CLR) begin
		if(CLR == 1'b0)
			VALUE <= 0;
		else if(CLK == 1'b1)
			VALUE = VALUE + 1'b1;
	end
	
endmodule