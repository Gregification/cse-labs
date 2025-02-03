`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/02/2025 02:28:31 PM
// Design Name: 
// Module Name: rv32_ex_alu32
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


module rv32_ex_alu32(
        input [31:0] a,
        input [31:0] b,
        input sub,
        
        output [31:0] out
    );
    
    // lab docs didnt mention anything about the ALUs internal logic
    assign out = (sub == 1) ? (a - b) : (a + b);
    
endmodule
