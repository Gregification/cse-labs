/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

module MuxSevSegController(
		input 					LOAD, 
		input 					MUX_CLK,
		input 					RESET,
		input	logic [15:0]	HEX,
		output logic[3:0]		CAT,
		output logic[6:0]		SEG
	);	
	
	logic [3:0] _currentBin;
	
	//acts as the 'PIPO' buffer
	logic [15:0] _binBuffer;
	
	always_ff @ (posedge LOAD, negedge RESET)
		if(RESET == 1'b0)
			_binBuffer = 16'd0;
		else if(LOAD == 1'b1)
			_binBuffer = HEX;
	
	//acts as the 'Four2One MUX'
	//	'CAT' value used as a alternative to 'SEL' since they do the same thing
	always 
		case (CAT)
			default	:	; //undefined
			4'b0001	:	_currentBin = _binBuffer[12+:4];
			4'b0010	:	_currentBin = _binBuffer[8+:4];
			4'b0100	:	_currentBin = _binBuffer[4+:4];
			4'b1000	:	_currentBin = _binBuffer[0+:4];
		endcase
	
	Bin2SevSegI __b2ss(
		.BIN(_currentBin),
		.INV(1),
		.SEG(SEG)
	);
	
	always_ff @ (posedge MUX_CLK, negedge RESET) begin
		if(RESET == 1'b0)
			CAT = 4'd0;
		else if(MUX_CLK == 1'b1) begin
			CAT = CAT << 1;
			
			if(CAT == 4'd0)
				CAT = 4'd1;
		end
	end

endmodule