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
        output reg wb_from_alu_out,
        output reg wb_from_io_out,

        // to pipe control
        output stall,

        // to mem interface
        output reg [31:2] memif_addr,
        input wire [31:0] memif_rdata, // already registered from memory
        output reg memif_we,
        output reg [3:0] memif_be,
        output reg [31:0] memif_wdata,

        // to io interface
        output reg [31:2] io_addr,
        input wire [31:0] io_rdata, // already registered from io
        output reg io_we,
        output reg [3:0] io_be,
        output reg [31:0] io_wdata
    );

    // some iw components
    wire        [6:0]  opcode  = iw_in[6:0];
    wire    [2:0]  funct3  = iw_in[14:12];

    always_ff @ (posedge clk) begin
        io_we <= 0;
        memif_we <= 0;
        wb_from_alu_out <= 1;

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

            memif_addr <= alu_in;
            io_addr <= alu_in;

            memif_wdata <= rs2_data_in;
            io_wdata <= rs2_data_in;

            // if is store operation
            if (opcode == 7'b0100011) begin
                if(iw_in[31] == 1) begin // is io address
                    io_we <= 1;
                    memif_we <= 0;
                end else begin // is memory address
                    io_we <= 0;
                    memif_we <= 1;
                end    
            end

            // if is load operation
            if (opcode == 7'b0000011) begin
                wb_from_alu_out <= 0;

                if(iw_in[31] == 1) begin // is io address
                    wb_from_io_out <= 1;
                end else begin // is memory address
                    wb_from_io_out <= 0;
                end    
            end

            case (funct3)
                3'b100,3'b000:
                    begin // LB : load byte U/S
                        io_be <= 4'b0001;
                        memif_be <= 4'b0001;
                    end
                3'b101,3'b001:
                    begin // LH : load halfword U/S
                        io_be <= 4'b0011;
                        memif_be <= 4'b0011;
                    end
                3'b010:begin // LW : load word signed
                        io_be <= 4'b1111;
                        memif_be <= 4'b1111;
                    end
                default: begin
                    io_be <= 4'b0000;
                    memif_be <= 4'b0000;
                end
            endcase

        end
    end
    
endmodule