module Bin2seven(
		input [3:0] BIN,
		output [0:6] SEV
    );
	 
    always
        case (BIN)
            4'd0	: SEV = 7'b1000000; 
            4'd1	: SEV = 7'b1111001; 
            4'd2	: SEV = 7'b0100100; 
            4'd3	: SEV = 7'b0110000; 
            4'd4	: SEV = 7'b0011001; 
            4'd5	: SEV = 7'b0010010; 
            4'd6	: SEV = 7'b0000010; 
            4'd7	: SEV = 7'b1111000; 
            4'd8	: SEV = 7'b0000000; 
            4'd9	: SEV = 7'b0011000; 
	    4'd10	: SEV = 7'b0001000; //A
	    4'd11	: SEV = 7'b0000011; //b
	    4'd12	: SEV = 7'b1000110; //C
	    4'd13	: SEV = 7'b0100001; //d
	    4'd14	: SEV = 7'b0000110; //E
	    4'd15	: SEV = 7'b0001110; //F
            default:SEV = 7'b0100011; //o
        endcase
	 
endmodule
