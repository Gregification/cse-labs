`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 05:25:44 PM
// Design Name: 
// Module Name: rv32_id_top
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

// instruciton decode
module rv32_id_top(
        // system clock and synchronous reset
        input clk,
        input reset,

        // from if
        input [31:0] pc_in,
        input [31:0] iw_in,

        // df from ex
        input [3:0] df_ex_wb_reg,
        input [31:0] df_ex_wb_data,
        input df_ex_wb_enable,
        // df from mem
        input [3:0] df_mem_wb_reg,
        input [31:0] df_mem_wb_data,
        input df_mem_wb_enable,
        // df from wb
        input [3:0] df_wb_wb_reg,
        input [31:0] df_wb_wb_data,
        input df_wb_wb_enable,

        // register interface
        output [4:0] regif_rs1_reg,
        output [4:0] regif_rs2_reg,
        input [31:0] regif_rs1_data_in,
        input [31:0] regif_rs2_data_in,

        // to ex
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out,
        output reg [31:0] regif_rs1_data_out,
        output reg [31:0] regif_rs2_data_out
    );

    wire [6:0] opcode;
    assign opcode   = iw_in[6:0];

    assign regif_rs1_reg = iw_in[19:15];
    assign regif_rs2_reg = iw_in[24:20];
    

    always_ff @ (posedge clk) begin
        if(reset) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;

            regif_rs1_data_out <= 0;
            regif_rs2_data_out <= 0;

        end else begin
            pc_out <= pc_in;
            iw_out <= iw_in;

            wb_reg_out      <= iw_in[11:7];
            wb_enable_out   <= 
                        opcode == 7'b0110011    // R type : orange
                    ||  opcode == 7'b0000011    // I type subsection : dull red
                    ||  opcode == 7'b0010011    // ^ : yellow
                    ||  opcode == 7'b0110111    // U type : dull blue
                    ||  opcode == 7'b0010111    // U type : dark green
                ;

            // if(df_ex_wb_enable && (df_ex_wb_reg == regif_rs1_reg))
            //     regif_rs1_data_out <= df_ex_wb_data;
            // else if(df_mem_wb_enable && (df_mem_wb_reg == regif_rs1_reg))
            //     regif_rs1_data_out <= df_mem_wb_data;
            // else if(df_wb_wb_enable && (df_wb_wb_reg == regif_rs1_reg))
            //     regif_rs1_data_out <= df_wb_wb_data;
            // else 
                regif_rs1_data_out <= regif_rs1_data_in;

            // if(df_ex_wb_enable && (df_ex_wb_reg == regif_rs2_reg))
            //     regif_rs2_data_out <= df_ex_wb_data;
            // else if(df_mem_wb_enable && (df_mem_wb_reg == regif_rs2_reg))
            //     regif_rs2_data_out <= df_mem_wb_data;
            // else if(df_wb_wb_enable && (df_wb_wb_reg == regif_rs2_reg))
            //     regif_rs2_data_out <= df_wb_wb_data;
            // else 
                regif_rs2_data_out <= regif_rs2_data_in;
        end
    end

endmodule
