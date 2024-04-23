/**
    lab 7
*/

/** doulbe digit 7 seg display
*/
module digitTo7Seg (
        input [4:0] NUM,
        output reg [7:0] SEG
    );
    always @ (NUM) begin
        case (NUM)
            4'b0000: SEG = 7'b0000001; //0
            4'b0001: SEG = 7'b1001111; //1
            4'b0010: SEG = 7'b0010010; //2
            4'b0011: SEG = 7'b0000110; //3
            4'b0100: SEG = 7'b1001100; //4
            4'b0101: SEG = 7'b0100100; //5
            4'b0110: SEG = 7'b0100000; //6
            4'b0111: SEG = 7'b0001111; //7
            4'b1000: SEG = 7'b0000000; //8
            4'b1001: SEG = 7'b0001100; //9
            default: SEG = 7'b1100010; //.
        endcase
        // SEG = ~SEG; //should work for active low
    end
endmodule

/** doulbe digit 7 seg display
*/
module secondsTo7Seg (
        input [5:0]NUM,
        output reg
            [7:0]SEG1,  //1s
            [7:0]SEG2   //10s
    );
    
    reg [5:0] mut;

    always @ (NUM) begin
        mut = NUM % 7'd60;

        digitTo7Seg tens(
            .NUM(mut / 7'd10),
            .SEG(SEG2),
        );

        digitTo7Seg ones(
            .NUM(mut % 7'd10),
            .SEG(SEG1),
        );

    end
endmodule

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

module OneHzClockToggle (
		input TOGGLE, CLOCK, RESET
		output ONEHz
	);
	
	OneHzClock OneHzClock_inst
	(
		.clock(CLOCK * TOGGLE) ,	// input  clock_sig
		.reset(RESET) ,	// input  reset_sig
		.OneHz(ONEHz) 	// output  OneHz_sig
	);
	
endmodule

//One Hz clock. Input clock is 50 MHz.
module OneHzClock //top module
    (
        input clock, reset,
        output OneHz,
    );

    wire TenMHz, OneMHz, OneKHz;
    
    //instantiations
    divideXn #(3'd5, 2'd3) div5 //module divideXn needs to be included only once in the project directory since it’s parameterized
    (
        .CLOCK(clock) , // input 50MHz clock
        .CLEAR(reset) , // input reset
        .OUT(TenMHz) , // output 10-MHz clock
        .COUNT(count) // output [3:0] count bits
    );

    divideXn #(4'd10, 3'd4) div10
    (
        .CLOCK(TenMHz) , // input 10MHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneMHz) , // output 1-MHz clock
        .COUNT(count) // output [3:0] count bits
    );
    
    divideXn #(10'd1000, 4'd10) div1000L
    (
        .CLOCK(OneMHz) , // input 1-MHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneKHz) , // output 1-KHz clock
        .COUNT(count) // output [3:0] count bits
    );
    
    divideXn #(10'd1000, 4'd10) div1000H
    (
        .CLOCK(OneKHz) , // input 1-KHz clock
        .CLEAR(reset) , // input reset
        .OUT(OneHz) , // output 1-Hz clock
        .COUNT(count) // output [3:0] count bits
    );
endmodule