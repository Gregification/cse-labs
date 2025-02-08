`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Student: George Boone , 1002055713
// 
// Create Date: 02/1/2025 11:43:52 AM
// Design Name: 
// Module Name: rv32_ex_top
// Project Name: cse4372 spring2025 lab2
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

// refrences
//  - ISA details: https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html#sb
//  - ISA excel sheet made by Jason Losh

module rv32_ex_top(
        // system clock and synchronous reset
        input clk,
        input reset,
        
        // from id
        input [31:0] pc_in,
        input [31:0] iw_in,         // instruction word
        input signed [31:0] rs1_data_in,
        input signed [31:0] rs2_data_in,
        
        // to mem
        output reg [31:0] alu_out
    );
    

    rv32_ex_alu32 alu32 (
        // from id
        .pc_in(pc_in),
        .iw_in(iw_in),
        .rs1_data_in(rs1_data_in),
        .rs2_data_in(rs2_data_in),
        
        // to mem
        .alu_out(alu_out)
    );
endmodule
