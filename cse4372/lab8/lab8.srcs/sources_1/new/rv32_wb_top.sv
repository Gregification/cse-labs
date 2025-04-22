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

        // from io/memory
        input [31:0] memif_rdata,
        input [31:0] io_rdata,
        input wb_from_alu_in,
        input [3:0] mem_be,

        // register interface
        output regif_wb_enable,
        output [4:0] regif_wb_reg,
        output [31:0] regif_wb_data
    );

    wire        [2:0]  funct3  = iw_in[14:12];

    wire isUnsigned;  // if loading unsigned value
    assign isUnsigned = (funct3 == 3'b101) || (funct3 == 3'b100);

    wire [31:0] shifted_mem_data;
    wire [31:0] mem_data;
    // assign shifted_mem_data = 
    //     (mem_be[3] || mem_be[2]) ? mem_data[31:0] :
    //     (mem_be[1]) ? 
    //         (
    //             isUnsigned ?   {16'b0           , mem_data[15:0]} :
    //                            {{8{mem_data[15]}}, mem_data[15:0]}
    //         ) :
    //     (mem_be[0]) ? 
    //         (
    //             isUnsigned ?   {{24'b0}         , mem_data[7:0]} :
    //                            {{24{mem_data[7]}}, mem_data[7:0]}
    //         ) : mem_data[31:0];
    assign shifted_mem_data = mem_data;

    assign regif_wb_enable = wb_enable_in;
    assign regif_wb_reg = wb_reg_in;
    assign regif_wb_data = wb_from_alu_in ? alu_in : shifted_mem_data;
    assign mem_data = iw_in[31] ? io_rdata : memif_rdata;

endmodule