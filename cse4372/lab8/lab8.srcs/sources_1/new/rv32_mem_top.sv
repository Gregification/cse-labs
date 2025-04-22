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

`include "defines.vh"

module rv32_mem_top(
        // system clock and synchronous reset
        input clk,
        input reset,
        
        // from ex
        input [31:0] pc_in,
        input [31:0] iw_in,
        input [31:0] alu_in,
        input [4:0] wb_reg_in,
        input wb_enable_in,
        input [31:0] rs2_data_in,

        // to wb
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [31:0] alu_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out,

        // memory interface for mem and io
        input [31:2] mem_addr_in,
        input [31:0] memif_rdata_in, // already registered from memory
        input [31:0] io_rdata_in, // already registered from io
        input memif_we_in,
        input io_we_in,
        input [3:0] mem_be_in,

        output reg [31:2] mem_addr_out,
        output wire [31:0] mem_wdata,
        output [31:0] memif_rdata_out, // already registered from memory
        output [31:0] io_rdata_out, // already registered from io
        output reg memif_we_out,
        output reg io_we_out,
        output reg [3:0] mem_be_out,
        output reg wb_from_alu_out
    );

    assign memif_addr = alu_in;
    assign io_addr = alu_in;
    assign mem_wdata = rs2_data_in;
    assign memif_rdata_out = memif_rdata_in;
    assign io_rdata_out = io_rdata_in;

    // some iw components
    wire        [6:0]  opcode  = iw_in[6:0];
    wire    [2:0]  funct3  = iw_in[14:12];

    always_ff @ (posedge clk) begin
        mem_addr_out <= mem_addr_in;
        memif_we_out <= memif_we_in;
        io_we_out <= io_we_in;
        mem_be_out <= mem_be_in;

        if(reset) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            alu_out <= 0;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;
            
        end else begin
            pc_out <= pc_in;
            iw_out <= iw_in;

            alu_out <= alu_in;

            wb_reg_out      <= wb_reg_in;
            wb_enable_out   <= wb_enable_in;

            if(opcode == 7'b0000011) // is load instruction
                wb_from_alu_out <= 0;
            else
                wb_from_alu_out <= 1;
        end
    end
    
endmodule