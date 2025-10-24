/**
parameters
NUM_REGS	: # of registers
REG_SIZE	: size of each register
ADDR_SIZE: size of address
*/

module Register_AxB#(parameter NUM_REGS = 1, REG_SIZE = 1, ADDR_SIZE = 1) (
		input CLK,	//clock
		input CLR,	//clear value at write address
		input WrEn,	//write enable
		
		input [REG_SIZE-1:0] DIN,	//data input
		output[REG_SIZE-1:0] DOUT,	//data output
		
		input	[ADDR_SIZE-1:0] RA,	//read address
		input	[ADDR_SIZE-1:0] WA	//write address
	);
	 
	//values stored here
	reg [REG_SIZE-1:0] _values [NUM_REGS-1:0];
		
	//constant readout for the read address
	assign DOUT = _values[RA];
	
	always_ff @ (posedge CLK, negedge CLR) begin
		//if clear triggered
		if(CLR == 1'b0)
			_values[WA] <= 0;
			
		//if clock triggered
		else if(CLK == 1'b1)
		
			//if am writing
			if(WrEn == 1'b0) begin
				//this zeroing then writing fixes the latency bug (at least thats what I think it is)
				_values[WA] = 0;
				_values[WA] = DIN; 
			end
	end
		

endmodule
