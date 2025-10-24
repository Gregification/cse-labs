/**
cse3341, digital logic 2, lab 3
George Boone
1002055713

a generic driver for the HexBoard.

main points
- is essentially a reskin of the "MuxSevSegController" from my lab2-partA
- registered logic
- hardcoded values specifically targeting the HexBoard
 
*/

module HexBoard(
		input	LOAD,
		input CLK,
		input RESET,
		input [15:0] 	VALUE,	//binary value
		
		output reg [3:0]	CAT,	//current toggled 7S display encoding
		output reg [6:0]	SEG	//current 7S encoding
	);

	reg [3:0]   _curr_digit_bin;	//binary value of the current digit
	reg [15:0]  _value_buffer;		//interal value register
	
	
	/**
	 * handles the registered logic.
	 * - buffers value on load.
	 * - clears on reset.
	 */
	always_ff @(posedge LOAD, negedge RESET)
		if(RESET == 1'b0)
			_value_buffer = 16'd0;
		else if(LOAD == 1'b1)
			_value_buffer = VALUE;
	
	/**
	 * keeps cat value as the relivent flag
	 * 	this oculd probably be some algebra equation but this is more intutive
	 */
	always
		case(CAT)
			default	:	; //undefined
			4'b0001	:	_curr_digit_bin = _value_buffer[12+:4];
			4'b0010	:	_curr_digit_bin = _value_buffer[8+:4];
			4'b0100	:	_curr_digit_bin = _value_buffer[4+:4];
			4'b1000	:	_curr_digit_bin = _value_buffer[0+:4];
		endcase
	
	/**
	 * 7s encoding
	 */
	Bin2SevSegI __bt7s(
		.BIN(_curr_digit_bin),
		.INV(1),
		.SEG(SEG)
	);
	
	/**
	 * go to next digit every clock
	 */
	always_ff @ (posedge CLK, negedge RESET) begin
		if(RESET == 0)
			CAT = 4'd0;
		else if(CLK == 1) begin
			CAT = CAT << 1;
			
			if(CAT == 4'd0)
				CAT = 4'd1;
		end
	end
	
endmodule
	