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
        output reg [31:0] regif_rs2_data_out,

        // to id : regarding pc jumping
        output jump_enable_out,
        output [31:0] jump_addr_out
    );

    wire [6:0] opcode;
    assign opcode   = iw_in[6:0];

    assign regif_rs1_reg = iw_in[19:15];
    assign regif_rs2_reg = iw_in[24:20];
    
    reg [31:0] regif_rs1_data;
    reg [31:0] regif_rs2_data;

    always_ff @ (posedge clk) begin
        // see step 4
        // check for JAL, JALR, and all the Break varients

        case (opcode)
            7'b1100111: begin // I type : dull blue
                // JALR : jump to address relative to register value
                // rd <- pc + 4 //TODO

                // pc
                /* page 8 of 2019 spec. 
                 *      "the JALR instruction now clears the lowest bit of the calculated target address,
                 *       to simplifyhardware and to allow auxiliary information to be stored in function
                 *       pointers."
                 */
                alu_out[31:0]   <= {{i_I + rs1_data_in}[31:1], 1'b0};
            end // I type : dull blue

            // 7'b0001111: begin // I type : white
            //     // FENCE     //TODO
            // end // I type : white

            // 7'b1110011: begin // I type : gray
            //     if(iw_in[20]) begin
            //         // ECALL     //TODO
            //     end else begin
            //         // EBREAK    //TODO
            //     end
            // end // I type : gray

            7'b1100011: begin // B type : white
                // case (funct3)
                    // 3'b000: // BEQ   //TODO
                    // 3'b001: // BNE   //TODO
                    // 3'b100: // BLT   //TODO
                    // 3'b101: // BGE   //TODO
                    // 3'b110: // BLTU  //TODO
                    // 3'b111: // BGEU  //TODO
                // endcase // funct3
            end // B type : white

            7'b1101111: begin // J type : ilme green
                alu_out <= $signed(pc_in + i_J); // JAL : jump to address relative to PC
            end // J type : ilme green
        endcase // opcode
    end

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

            regif_rs1_data_out <= regif_rs1_data;
            regif_rs2_data_out <= regif_rs2_data;
        end
    end

    always_comb begin
        if(df_ex_wb_enable && (df_ex_wb_reg == regif_rs1_reg))
            regif_rs1_data <= df_ex_wb_data;
        else if(df_mem_wb_enable && (df_mem_wb_reg == regif_rs1_reg))
            regif_rs1_data <= df_mem_wb_data;
        else if(df_wb_wb_enable && (df_wb_wb_reg == regif_rs1_reg))
            regif_rs1_data <= df_wb_wb_data;
        else 
            regif_rs1_data <= regif_rs1_data_in;

        if(df_ex_wb_enable && (df_ex_wb_reg == regif_rs2_reg))
            regif_rs2_data <= df_ex_wb_data;
        else if(df_mem_wb_enable && (df_mem_wb_reg == regif_rs2_reg))
            regif_rs2_data <= df_mem_wb_data;
        else if(df_wb_wb_enable && (df_wb_wb_reg == regif_rs2_reg))
            regif_rs2_data <= df_wb_wb_data;
        else 
            regif_rs2_data <= regif_rs2_data_in;        
    end

endmodule