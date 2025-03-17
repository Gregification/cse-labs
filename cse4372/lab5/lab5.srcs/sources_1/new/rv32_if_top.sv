`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 05:23:17 PM
// Design Name: 
// Module Name: rv32_if_top
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

// instruction fetch
module rv32_if_top(
        // system clock and synchronous reset
        input clk,
        input reset,

        // memory interface
        output [31:2] memif_addr,
        input [31:0] memif_data,

        // to id
        output reg [31:0] pc_out,
        output [31:0] iw_out        // note: alrealy registerd in memory
    );
endmodule
