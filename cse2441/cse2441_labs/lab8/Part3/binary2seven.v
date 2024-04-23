module binary2seven (
        input [3:0] BIN,
        output reg [7:0] SEV
    );
    always @ (BIN) begin
        case (BIN)
            4'd0: SEV = 7'b0000001; 
            4'd1: SEV = 7'b1001111; 
            4'd2: SEV = 7'b0010010; 
            4'd3: SEV = 7'b0000110; 
            4'd4: SEV = 7'b1001100; 
            4'd5: SEV = 7'b0100100; 
            4'd6: SEV = 7'b0100000; 
            4'd7: SEV = 7'b0001111; 
            4'd8: SEV = 7'b0000000; 
            4'd9: SEV = 7'b0001100; 
            default: SEV = 7'b1100010; //.
        endcase
        // SEG = ~SEG; //should work for active low
    end
endmodule