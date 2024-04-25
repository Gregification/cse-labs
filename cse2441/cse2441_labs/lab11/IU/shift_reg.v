module shift_reg #(
        parameter COUNT = 4,
        parameter WIDTH = 4
    ) (
        input trig, reset, dir, // Left = 0, Right = 1
        input [WIDTH-1:0] in,
        output reg [(COUNT*WIDTH)-1:0] out
    );
    always @ (posedge trig, negedge reset) begin
        if (~reset)
            out <= 0;
        else if((in <= 9 || in == 4'hE) && in != 4'hA && in != 4'hB && in != 4'hC) begin		  
            if (dir) begin // 1 = Right
                out <= out >> WIDTH;
                out[(COUNT*WIDTH)-1:((COUNT*WIDTH)-1)-WIDTH] <= in;
            end
            else begin // 0 = Left
                out <= out << WIDTH;
                out[WIDTH-1:0] <= in;
            end
        end
    end
endmodule