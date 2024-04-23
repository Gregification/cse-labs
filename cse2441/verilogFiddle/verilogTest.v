/**
    > DE10-Lite 7-seg dispalys are active low

    3'b101  : three-bit binary number 101
    8'd101  : eight-bit decimal number 101
    8'h9e   : eight-bit hexadecimal number 9E

    =       : blocking assignment
    <=      : nonblocking assignment

    : concatenation exmaple
        # if a = 0, bi = 1, cini = 0, then {ai,bi,cini} = 010

    : procedual block
        initial     : init
        alwayse     : continuously

    : alwayse with or condition
        alwayse @ (a or b)

    : if statements
        # if (condition1) statement1;
        # else if(condition2) statement2;
        # else defaultStatement;
*/

////////////////////////////
//realization of f(x,y,z)= y'z' + yz + xz
////////////////////////////
/**dataflow model*/
module ExampleF (x, y, z, f);
    input x, y, z;
    output f;
    assign f = ~y & ~z | y & z | x & z;
endmodule

/**structural model*/
module ExampleS (x,y,z,f);
    input x,y,z;
    output f;
    wire a,b,c;
        nand (a, ~y, ~z);
        nand (b, y, z);
        nand (c, x, y);
        nand (f, a, b, c);
endmodule

////////////////////////////
//even parity encoder
////////////////////////////

module evenParityEncoderDF(
        input [7:0] d,
        output P
    );
    assign P = d[7]^d[6]^d[5]^d[4]^d[3]^d[2]^d[1]^d[0];
endmodule

////////////////////////////
//adders
////////////////////////////
/** full adder dataflow*/
module FA_DF (si, couti, ai, bi, cini);
    input ai, bi, cini;
    output si, cout;

    assign si = ai ^ bi ^ cini;
    assign couti = ai & bi | ai & cini | bi & cini;
endmodule

/**full adder structural*/
module FAstruct (si, couti, ai, bi, cini);
    input ai, bi, cini;
    output si, couti;

    wire d,e,f,g;
        xor (d, ai, bi);
        xor (si, d, cini);
        and (e, ai, bi);
        and (f, ai, cini);
        and (g, bi, cini);
        or (couti, e, f, g);
endmodule

/** full adder behavorial */
module FAbehav (
        input ai, bi, cini,
        output reg si, couti
    );
    always @(ai, bi cini) begin
        case ({ai, bi, cini})
            3'b000: begin couti = 1'b0; si = 1'b0; end
            3'b001: begin couti = 1'b0; si = 1'b1; end
            3'b010: begin couti = 1'b0; si = 1'b1; end
            3'b011: begin couti = 1'b1; si = 1'b0; end
            3'b100: begin couti = 1'b0; si = 1'b1; end
            3'b101: begin couti = 1'b1; si = 1'b0; end
            3'b110: begin couti = 1'b1; si = 1'b0; end
            3'b111: begin couti = 1'b1; si = 1'b1; end
        endcase
    end
endmodule

/** ripple carry adder using full adder*/
module RippleCarryAdderStructural (
        input  [3:0] A, B,
        output [3:0] S,
        output Cout
    );
    wire [4:0] C;
    assign C[0] = 1'b0;
   
    FAbehav s0 (.ai(A[0]), .bi(B[0]), .cini(C[0]), .si(S[0]), .couti(C[1]));
    FAbehav s1 (A[1], B[1], C[1], S[1], C[2]);
    FAbehav s2 (A[2], B[2], C[2], S[2], C[3]);
    FAbehav s3 (A[3], B[3], C[3], S[3], C[4]);
   
    assign Cout = C[4];
endmodule

//Verilog Model of an N-bit register with active-low asynchronous clear
module NBitRegister #(parameter N = 4)(
        input [N-1:0] D, //declare N-bit data input
        input CLK, CLR, //declare clock and clear inputs
        output reg [N-1:0] Q
    ); //declare N-bit data output

    always @ (posedge CLK, negedge CLR) begin //detect change of clock or clear
        if (CLR==1'b0) Q <= 0; //register loaded with all 0’s
        else if (CLK==1'b1) Q <= D; //data input values loaded in register
    end
endmodule

//full adder
module FullAdder(
        input A, B, C,
        output reg R, COUT  
    );

    wire xor1, and1, and2, and3;

    always @ (A,B,C) begin
        xor(xor1, A, B);
        xor(R, C, xor1);

        and(and1, B, A);
        and(and2, B, C);
        and(and3, A, C);
        and(COUT, and1, and2, and3);
    end
endmodule

//Ripple Carry Adder Structural Model
module RippleCarryAdderStructural ( //name the module
        input [7:0] A, B,
        input add_subtract, //declare input ports
        output [7:0] S, //declare output ports for sum
        output Cout
    ); //declare carry-out port
    
    wire [8:0] C; //declare internal nets
    assign C[0] = Add_subtract;
    assign Cout = C[8]; //rename carry-out port
    
    //instantiate the full adder module for each stage of the ripple carry adder/subtractor
    FAbehav s0 (A[0], B[0]^C[0], C[0], S[0], C[1]);
    FAbehav s1 (A[1], B[1]^C[0], C[1], S[1], C[2]);
    FAbehav s2 (A[2], B[2]^C[0], C[2], S[2], C[3]);
    FAbehav s3 (A[3], B[3]^C[0], C[3], S[3], C[4]);
    FAbehav s4 (A[4], B[4]^C[0], C[4], S[4], C[5]);
    FAbehav s5 (A[5], B[5]^C[0], C[5], S[5], C[6]);
    FAbehav s6 (A[6], B[6]^C[0], C[6], S[6], C[7]);
    FAbehav s7 (A[7], B[7]^C[0], C[7], S[7], C[8]);
endmodule

module IU(
        input CLK,
        input RESET,
        input [3:0] ROWS,
        output [3:0] COLS,

        output ISVALID,
        output [7:0] BinTC;
    );

    wire [15:0] BCD;
    wire [7:0] BinSM;

    keypad_input keyIN (
        .clk(CLK),
        .reset(RESET),
        .row(ROWS),
        .col(COLS),
        .out(BDC)
    );

    BCD2BinarySM bcd2BinSM {
        .BCD(BCD),
        .binarySM(BinSM)
    };

    assign ISVALID  = BinSM[6:0] <= 127;
    assign BinTC    = BinSM[7] ? ~BinSM + 1 : BinSM;

endmodule

module OU(
        input[7:0] BinTC
    );

    wire[7:0] BMS;

    twoSIGN binTC2SM(
        .A(BinTC),
        .B(BSM)
    );

endmodule