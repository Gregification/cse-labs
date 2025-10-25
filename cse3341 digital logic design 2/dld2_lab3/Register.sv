

module Register #(
		parameter SIZE = 4
	)(
		input TRIGGER,
		input CLEAR,
		input [SIZE-1:0] VALUE_IN,
		
		output reg [SIZE-1:0] VALUE_OUT
	);
	
	always_ff @ (posedge TRIGGER, negedge CLEAR) begin
		if(CLEAR == 1'b0) 
			VALUE_OUT = 0;
		else if(TRIGGER == 1'b1)
			VALUE_OUT = VALUE_IN;
	end
	
endmodule
