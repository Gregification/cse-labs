
//One Hz clock. Input clock is 50 MHz.
module OneHzClock (
        input clock, reset,
        output OneHz
    );

    wire TenMHz, OneMHz, OneKHz;
    
    //instantiations
    counter #(3'd5, 2'd3) div5 //module divideXn needs to be included only once in the project directory since it’s parameterized
    (
        .CLOCK(clock) , // input 50MHz clock
        .CLEAR(reset) , // input reset
        .OUT(TenMHz) , // output 10-MHz clock
        .COUNT(count) // output [3:0] count bits
    );

    counter #(4'd10, 3'd4) div10
    (
        .CLOCK(TenMHz) , // input 10MHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneMHz) , // output 1-MHz clock
        .COUNT(count) // output [3:0] count bits
    );
    
    counter #(10'd100, 4'd10) div1000L
    (
        .CLOCK(OneMHz) , // input 1-MHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneKHz) , // output 1-KHz clock
        .COUNT(count) // output [3:0] count bits
    );
    
    counter #(10'd200, 4'd10) div1000H
    (
        .CLOCK(OneKHz) , // input 1-KHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneHz) , // output 1-Hz clock
        .COUNT(count) // output [3:0] count bits
    );
endmodule