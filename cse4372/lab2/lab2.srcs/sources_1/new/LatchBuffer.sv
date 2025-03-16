`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/08/2025 12:01:25 PM
// Design Name: 
// Module Name: LatchBuffer
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

// this has nothing to do with latches, i just name it that; its a hassle to rename.
// indended to handle input metastability saftely
module LatchBuffer#(
        parameter WIDTH = 1
    )(
        input wire clk,
        input wire [WIDTH-1:0] sig_in,
        
        output reg [WIDTH-1:0] sig_out
    );

    reg [WIDTH-1:0] buff;
    always_ff @ (posedge(clk)) begin
        buff[WIDTH-1:0]     <= sig_in[WIDTH-1:0];
        sig_out[WIDTH-1:0]  <= buff[WIDTH-1:0];
    end

endmodule
