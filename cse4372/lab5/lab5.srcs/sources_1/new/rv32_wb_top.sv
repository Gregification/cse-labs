`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 06:15:19 PM
// Design Name: 
// Module Name: rv32_wb_top
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


module rv32_wb_top(
        // system clock and synchronous reset
        input clk,
        input reset,

        // from mem
        input [31:0] pc_in,
        input [31:0] iw_in,
        input [31:0] alu_in,
        input [4:0] wb_reg_in,
        input wb_enable_in,

        // register interface
        output regif_wb_enable,
        output [4:0] regif_wb_reg,
        output [31:0] regif_wb_data
    );

    assign regif_wb_enable = wb_enable_in;
    assign regif_wb_reg = wb_reg_in;
    assign regif_wb_data = alu_in;

endmodule
