//lecture slides 11.1   
        
//BCD to seven-segment decoder. Active low output.
module bcd2seven (
    input w,x,y,z,
    output reg a,b,c,d,e,f,g);
    always @ (w,x,y,z) begin
        case ({w,x,y,z})
            4'b0000: {a,b,c,d,e,f,g} = 7'b0000001; //0
            4'b0001: {a,b,c,d,e,f,g} = 7'b1001111; //1
            4'b0010: {a,b,c,d,e,f,g} = 7'b0010010; //2
            4'b0011: {a,b,c,d,e,f,g} = 7'b0000110; //3
            4'b0100: {a,b,c,d,e,f,g} = 7'b1001100; //4
            4'b0101: {a,b,c,d,e,f,g} = 7'b0100100; //5
            4'b0110: {a,b,c,d,e,f,g} = 7'b0100000; //6
            4'b0111: {a,b,c,d,e,f,g} = 7'b0001111; //7
            4’b1000: {a,b,c,d,e,f,g} = 7'b0000000; //8
            4’b1001: {a,b,c,d,e,f,g} = 7'b0001100; //9
            default: {a,b,c,d,e,f,g} = 7’b1111111; //blank
        endcase
    end
endmodule