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
        output wire [31:2] memif_addr,
        input [31:0] memif_data,

        // to id
        output wire [31:0] pc_out,
        output [31:0] iw_out,       // note: alrealy registerd in memory

        // from id
        input wire jump_enable_in,
        input wire [31:0] jump_addr_in,

        input stall
    );

    reg [31:0] pc;
    initial pc = `PC_RESET;

    assign memif_addr = pc_out[31:2];

    assign iw_out = memif_data; // memif_data is already registered comming out of memory

    reg stall_former;
    initial stall_former = 0;

    always_ff @ (posedge clk) begin
        stall_former <= stall;
        // itterate pc
        if(reset) begin
            pc <= `PC_RESET;
            stall_former <= 0;
        end if( stall ) begin
            if(!stall_former) begin
                pc <= pc - 4;
            end
            // do nothing
        end else if(jump_enable_in) begin
            pc <= jump_addr_in;
        end else begin
            pc <= pc + 4;
        end

        // scuffed way to handle stalling.
        //      requied or else the next instruciton after the stall is executed twice
        //      because it'll take one clock to get out, and another to itterate -
        //      all while the memeory already has the instruciton loaded into execute
        if(stall_former && !stall)
            pc <= pc + 8;
    end

    assign pc_out = reset ? `PC_RESET : jump_enable_in ? (jump_addr_in - 4) : (stall_former && !stall) ? (pc+4) : pc;
    
endmodule