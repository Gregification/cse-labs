module BCD2BinarySM #(parameter N=8) (
        input [15:0] BCD,
        output [N-1:0] binarySM
    );
    assign binarySM = BCD[15]*(8'b10000000) + BCD[11:8]*(8'b01100100) + BCD[7:4]*(8'b1010) + BCD[3:0];
endmodule