/** 4 bit binary to 7 segment encoder. 
    - dec output
    - active low

    @Param z least significant
    @Param w most significant
    @Return 1-9 , blank otherwise
*/
module signed_bit4To7segX2_dec(
        input w,x,y,z,
        output reg
            a1,b1,c1,d1,e1,f1,g1,   //1
            a2,b2,c2,d2,e2,f2,g2    //10
    );
    always @ (w,x,y,z) begin
        //1's
        case ({w,x,y,z})
            4'b0000: {a1,b1,c1,d1,e1,f1,g1} = 7'b0000001; //0
            4'b0001: {a1,b1,c1,d1,e1,f1,g1} = 7'b1001111; //1
            4'b0010: {a1,b1,c1,d1,e1,f1,g1} = 7'b0010010; //2
            4'b0011: {a1,b1,c1,d1,e1,f1,g1} = 7'b0000110; //3
            4'b0100: {a1,b1,c1,d1,e1,f1,g1} = 7'b1001100; //4
            4'b0101: {a1,b1,c1,d1,e1,f1,g1} = 7'b0100100; //5
            4'b0110: {a1,b1,c1,d1,e1,f1,g1} = 7'b0100000; //6
            4'b0111: {a1,b1,c1,d1,e1,f1,g1} = 7'b0001111; //7
            4'b1000: {a1,b1,c1,d1,e1,f1,g1} = 7'b0000000; //-8
            4'b1001: {a1,b1,c1,d1,e1,f1,g1} = 7'b0001111; //-7
            4'b1010: {a1,b1,c1,d1,e1,f1,g1} = 7'b0100000; //-6
            4'b1011: {a1,b1,c1,d1,e1,f1,g1} = 7'b0100100; //-5
            4'b1100: {a1,b1,c1,d1,e1,f1,g1} = 7'b1001100; //-4
            4'b1101: {a1,b1,c1,d1,e1,f1,g1} = 7'b0000110; //-3
            4'b1110: {a1,b1,c1,d1,e1,f1,g1} = 7'b0010010; //-2
            4'b1111: {a1,b1,c1,d1,e1,f1,g1} = 7'b1001111; //-1
        endcase

        //10's
        if (w == 1)
            {a2,b2,c2,d2,e2,f2,g2} = 7'b1111110; //neg -> 1
        else 
            {a2,b2,c2,d2,e2,f2,g2} = 7'b1111111; // pos -> [blank]
    end
endmodule