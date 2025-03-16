// (NOT TOP LEVEL)
// Binary to Seven Segment Display
// This module should be instantiated three times in the top level

//////////////////////////////////////////////////////
//
// MODIFY THIS FILE
// Goal 1: Combine all single seperate inputs into one 4-bit vector
//         input w,x,y,z -> input [N-1:0] BIN, where N is bit length
//
// Goal 2: Combine all single seperate outputs into one 7-bit vector
//         output reg a,b,c,d,e,f,g -> output reg [0:N-1] SEV, where N is bit length
//
//
//////////////////////////////////////////////////////

module binary2seven(
		input [3:0] BIN,
        output reg [0:6] SEV
    );
    always @ (BIN) begin
        case (BIN)
            4'd0	: SEV = 7'b0000001; 
            4'd1	: SEV = 7'b1001111; 
            4'd2	: SEV = 7'b0010010; 
            4'd3	: SEV = 7'b0000110; 
            4'd4	: SEV = 7'b1001100; 
            4'd5	: SEV = 7'b0100100; 
            4'd6	: SEV = 7'b0100000; 
            4'd7	: SEV = 7'b0001111; 
            4'd8	: SEV = 7'b0000000; 
            4'd9	: SEV = 7'b0001100; 
				4'd10	: SEV = 7'b0001000; //A
				4'd11	: SEV = 7'b1100000; //b
				4'd12	: SEV = 7'b0110001; //C
				4'd13	: SEV = 7'b1000010; //d
				4'd14	: SEV = 7'b0110000; //E
				4'd15	: SEV = 7'b0111000; //F
            default:SEV = 7'b1100010; //.
        endcase
	  end
endmodule
