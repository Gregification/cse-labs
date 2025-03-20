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

// instruciton decode
module rv32_id_top(
        // system clock and synchronous reset
        input clk,
        input reset,

        // from if
        input [31:0] pc_in,
        input [31:0] iw_in,
        output [4:0] regif_rs1_reg,
        output [4:0] regif_rs2_reg,
        input [31:0] regif_rs1_data,
        input [31:0] regif_rs2_data,

        // to ex
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out
    );

    always_ff @ (posedge clk) begin
        pc_out <= pc_in;
        iw_out <= iw_in;
    end

endmodule
