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
        input memif_we_in,
        input io_we_in,
        input [3:0] mem_be_in,

        //to id
        output [31:0] alu_raw,

        // to mem
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [31:0] alu_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out,
        output reg [31:0] rs2_data_out,
        output reg [31:2] memif_addr_out,
        output reg memif_we_out,
        output reg io_we_out,
        output reg [3:0] mem_be_out,
        
        // register df from wb (from mem_read)
        input df_wb_from_mem_wb,
        input [4:0] df_wb_reg,
        input [31:0] df_wb_data
    );

    always_ff @ (posedge(clk)) begin
        if(reset) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            alu_out <= 0;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;

            memif_addr_out <= 0;
            memif_we_out <= 0;
            io_we_out <= 0;
            mem_be_out <= 0;
        end else begin
            pc_out <= pc_in;
            iw_out <= iw_in;

            alu_out <= _alu32.alu_out;

            wb_reg_out      <= wb_reg_in;
            wb_enable_out   <= wb_enable_in;

            memif_addr_out <= _alu32.alu_out[31:2];
            memif_we_out <= memif_we_in;
            io_we_out <= io_we_in;
            mem_be_out <= mem_be_in;
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