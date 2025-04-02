//divide by 5
//divide by 10. slide 10 of lecture 9.2
//divide by 1000 is just N=1000, M=10
// Divide by N counter with M state variables
module divideXn #(parameter N=5, parameter M=3)
    (
        input CLOCK, CLEAR,
        output reg [M-1:0] COUNT, // COUNT is defined as a M-bit register
        output reg OUT
    );

    always @ (negedge CLOCK, negedge CLEAR)
        if (CLEAR==1'b0) COUNT <= 0; // COUNT is loaded with all 0's
        else
        begin
            if (COUNT == N-2'd2) begin OUT <= 1'b1; COUNT <= N-1'd1; end // Once COUNT = N-2 OUT = 1
            else
            if (COUNT == N-1'd1) begin OUT <=1'b0; COUNT <= 0; end //Once COUNT = N-1 OUT=0
            else begin OUT <= 1'b0; COUNT <= COUNT + 1'b1; end // COUNT is incremented
        end
endmodule