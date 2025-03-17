`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 06:12:06 PM
// Design Name: 
// Module Name: rv32_mem_top
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


module rv32_mem_top(
        // system clock and synchronous reset
        input clk,
        input reset,
        
        // from ex
        input [31:0] pc_in,
        input [3:0] wb_reg_in,
        input wb_enable_in,

        // to wb
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out
    );
endmodule
