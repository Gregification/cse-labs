module decadeCounter(
        input CLK,
        input RESET,
        output [3:0] value
    );

    always @ (posedge CLK, negedge RESET) begin
        if(RESET == 1'b0) value = 1'b0;
        else if(CLK == 1'b1) begin
            value <= value + 1;
            
            if(value > 9) value <= 1'b0;
        end
    end

endmodule