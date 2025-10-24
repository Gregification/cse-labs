/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

/**
Binary To Seven Segment Invertable

a invertable  4 bit to 7-seg hex value multiplexer.

- defaults active low
- undefined results in a lowercae letter 'O'

(pretty much identical to normal bin2sev)
*/
module Bin2SevSegI(
		input 	[3:0] BIN,	//binary
		input				INV,	//invert? 1:yes, 0:no
		output	[0:6] SEG	//segments
	);
	
	//idk if this can just be a wire, dosen't need to store a value, just wire XOR with INV, but this is eaier to code 
	logic [0:6] _buffer;
	
	//always SEG = _buffer;
	always SEG = INV ? ~_buffer : _buffer;
	
	always
		case (BIN)
            4'd0	: _buffer = 7'b1000000; 
            4'd1	: _buffer = 7'b1111001; 
            4'd2	: _buffer = 7'b0100100; 
            4'd3	: _buffer = 7'b0110000; 
            4'd4	: _buffer = 7'b0011001; 
            4'd5	: _buffer = 7'b0010010; 
            4'd6	: _buffer = 7'b0000010; 
            4'd7	: _buffer = 7'b1111000; 
            4'd8	: _buffer = 7'b0000000; 
            4'd9	: _buffer = 7'b0011000; 
				4'd10	: _buffer = 7'b0001000; //A
				4'd11	: _buffer = 7'b0000011; //b
				4'd12	: _buffer = 7'b1000110; //C
				4'd13	: _buffer = 7'b0100001; //d
				4'd14	: _buffer = 7'b0000110; //E
				4'd15	: _buffer = 7'b0001110; //F
            default:_buffer = 7'b0100011; //o
        endcase

endmodule