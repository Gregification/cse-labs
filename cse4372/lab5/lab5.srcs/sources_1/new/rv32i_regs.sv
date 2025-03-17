`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 04:54:52 PM
// Design Name: 
// Module Name: rv32i_regs
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


module rv32i_regs(
        // system clock and synchronous reset
        input clk,
        input reset,

        // inputs
        input [4:0] rs1_reg,
        input [4:0] rs2_reg,
        input wb_enable,
        input [4:0] wb_reg,
        input [31:0] wb_data,

        // outputs
        output [31:0] rs1_data,
        output [31:0] rs2_data
    );
    
    reg     [31:0][31:0] registers;

    always_ff @ (posedge(clk)) begin
        if (reset) begin

            registers <= '{default : '0};

        end else begin

            if (wb_enable) begin
                registers[wb_reg] <= wb_data;
            end
            
        end

        // register[0] hardwired to zero
        registers[0] <= 0;
    end

    // async read out of rs1,2
    assign rs1_data = registers[rs1_reg];
    assign rs2_data = registers[rs2_reg];

endmodule
