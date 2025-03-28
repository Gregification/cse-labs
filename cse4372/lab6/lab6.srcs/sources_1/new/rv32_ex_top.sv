`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 05:28:39 PM
// Design Name: 
// Module Name: rv32_ex_top
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

`include "defines.vh"

module rv32_ex_top(
        // system clock and synchronous reset
        input clk,
        input reset,
        
        // from id
        input [31:0] pc_in,
        input [31:0] iw_in,
        input [31:0] rs1_data_in,
        input [31:0] rs2_data_in,
        input [4:0] wb_reg_in,
        input wb_enable_in,

        //to id
        output [31:0] alu_raw,

        // to mem
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [31:0] alu_out,
        output reg [3:0] wb_reg_out,
        output reg wb_enable_out
    );

    always_ff @ (posedge(clk)) begin
        if(reset) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            alu_out <= 0;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;
        end else begin
            pc_out <= pc_in;
            iw_out <= iw_in;

            alu_out <= _alu32.alu_out;

            wb_reg_out      <= wb_reg_in;
            wb_enable_out   <= wb_enable_in;
        end
    end
    
    rv32_ex_alu32 _alu32 (
        // from id
        .pc_in(pc_in),
        .iw_in(iw_in),
        .rs1_data_in(rs1_data_in),
        .rs2_data_in(rs2_data_in)
        
        // to mem
        // output reg [31:0] alu_out
    );

    assign alu_raw = _alu32.alu_out;
endmodule