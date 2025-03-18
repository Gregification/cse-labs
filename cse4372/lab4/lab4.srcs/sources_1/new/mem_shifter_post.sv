`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/08/2025 01:39:37 PM
// Design Name: 
// Module Name: mem_shifter_post
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


module mem_shifter_post(
        input clk,
        input [31:0] d_rdata,
        output reg [31:0] d_rdata_shifted,

        input isUnsigned,
        input [3:0] d_be
    );

    always_ff @ (posedge clk) begin
        if(d_be[3] || d_be[2]) begin 
            d_rdata_shifted <= d_rdata[31:0];
        end else if(d_be[1]) begin
            if(isUnsigned)
                d_rdata_shifted <= {16'b0           , d_rdata[15:0]};
            else
                d_rdata_shifted <= {{8{d_rdata[15]}}, d_rdata[15:0]};
        end else if(d_be[0]) begin
            if(isUnsigned)
                d_rdata_shifted <= {{24'b0}         , d_rdata[7:0]};
            else
                d_rdata_shifted <= {{24{d_rdata[7]}}, d_rdata[7:0]};
        end else begin
            d_rdata_shifted <= d_rdata[31:0];
        end
    end

endmodule
