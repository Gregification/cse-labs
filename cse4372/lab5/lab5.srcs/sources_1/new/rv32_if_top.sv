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

`include "defines.vh"

// instruction fetch
/**
 * itterates a buffered PC so as to allow the memeory 1 clock cycle to fetch the next instruction 
 */
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

    reg [31:0] pc;
    assign memif_addr = pc[31:2];

    assign iw_out = memif_data; // memif_data is already registered comming out of memory

    always_ff @ (posedge clk) begin
        pc_out <= pc; // pc_out gets former pc value, since pc is registered

        // itterate pc
        if(reset) begin
            pc <= `PC_RESET;
        end else begin
            pc <= pc + 4;
        end
    end

endmodule
