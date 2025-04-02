`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/18/2025 09:14:02 PM
// Design Name: 
// Module Name: SynchBuffer
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


module SynchBuffer#(
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
