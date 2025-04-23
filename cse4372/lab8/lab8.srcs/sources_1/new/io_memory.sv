`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/22/2025 09:08:44 PM
// Design Name: 
// Module Name: io_memory
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

/**
faked single port memory
*/
module io_memory(
        input clk,

        // access
        input [31:2] addr,
        input we,
        input [31:0] wdata,
        output reg [31:0] rdata,
        input [3:0] be,

        // top routing outputs
        output reg [9:0] led_out,

        // top routing inputs
        input reg [3:0] pb_in,
        input reg [11:0] sw_in,

        // debugging
        output wire [31:0] offset
    );

    assign offset = addr[20:2];

    `define LED_OFFSET 0
    `define PB_OFFSET 1
    `define SW_OFFSET 2

    always_ff @ (posedge clk) begin
        rdata <= 0;

        // seperate logic for R/W operations

        if (we) begin // W
            case (offset)
                `LED_OFFSET: begin
                    led_out <= wdata[9:0];
                    rdata <= 32'hbeebebee;
                end

                default: rdata <= 32'hDEADBEEE;
            endcase
        end else begin // R
            case (offset)
                `LED_OFFSET : rdata <= led_out;
                `PB_OFFSET : rdata <= pb_in;
                `SW_OFFSET : rdata <= sw_in;

                default: rdata <= 32'hDEADBEEF;
            endcase
        end
    end

endmodule
