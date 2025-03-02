`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/01/2025 05:23:07 PM
// Design Name: 
// Module Name: dual_port_ram
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

`define ADDR_WIDTH 15


module dual_port_ram(
        // Clock
        input clk,
        
        // Instruction port (RO)
        input [31:2] i_addr,
        output reg [31:0] i_rdata,
        
        // Data port (RW)
        input [31:2] d_addr,
        output reg [31:0] d_rdata,
        input d_we,
        input [3:0] d_be,
        input [31:0] d_wdata
    );
    
    reg [31:0] mem [2**`ADDR_WIDTH-1:0];
    
    always @(posedge clk)
        i_rdata <= mem[i_addr];
        
    always @(posedge clk) begin
        if (d_we) begin
            if(d_be[0]) mem[d_addr][7:0]    <= d_wdata[7:0];
            if(d_be[1]) mem[d_addr][15:8]   <= d_wdata[15:8];
            if(d_be[2]) mem[d_addr][23:16]  <= d_wdata[23:16];
            if(d_be[3]) mem[d_addr][31:24]  <= d_wdata[31:24];
        end else
            d_rdata <= mem[d_addr];
    end

endmodule
