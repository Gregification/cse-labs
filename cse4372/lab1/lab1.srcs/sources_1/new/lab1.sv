`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 01/29/2025 10:09:40 AM
// Design Name: 
// Module Name: lab1
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


module lab1(
    input CLK100,           // 100 MHz clock input
    output [9:0] LED,       // RGB1, RGB0, LED 9..0 placed from left to right
    output [2:0] RGB0,      
    output [2:0] RGB1,
    output [3:0] SS_ANODE,   // Anodes 3..0 placed from left to right
    output [7:0] SS_CATHODE, // Bit order: DP, G, F, E, D, C, B, A
    input [11:0] SW,         // SWs 11..0 placed from left to right
    input [3:0] PB,          // PBs 3..0 placed from left to right
    inout [23:0] GPIO,       // PMODA-C 1P, 1N, ... 3P, 3N order
    output [3:0] SERVO,      // Servo outputs
    output PDM_SPEAKER,      // PDM signals for mic and speaker
    input PDM_MIC_DATA,      
    output PDM_MIC_CLK,
    output ESP32_UART1_TXD,  // WiFi/Bluetooth serial interface 1
    input ESP32_UART1_RXD,
    output IMU_SCLK,         // IMU spi clk
    output IMU_SDI,          // IMU spi data input
    input IMU_SDO_AG,        // IMU spi data output (accel/gyro)
    input IMU_SDO_M,         // IMU spi data output (mag)
    output IMU_CS_AG,        // IMU cs (accel/gyro) 
    output IMU_CS_M,         // IMU cs (mag)
    input IMU_DRDY_M,        // IMU data ready (mag)
    input IMU_INT1_AG,       // IMU interrupt (accel/gyro)
    input IMU_INT_M,         // IMU interrupt (mag)
    output IMU_DEN_AG        // IMU data enable (accel/gyro)
    );
     
    // Terminate all of the unused outputs or i/o's
    // assign LED = 10'b0000000000;
    assign RGB0 = 3'b000;
    assign RGB1 = 3'b000;
    // assign SS_ANODE = 4'b0000;
    // assign SS_CATHODE = 8'b11111111;
    assign GPIO = 24'bzzzzzzzzzzzzzzzzzzzzzzzz;
    assign SERVO = 4'b0000;
    assign PDM_SPEAKER = 1'b0;
    assign PDM_MIC_CLK = 1'b0;
    assign ESP32_UART1_TXD = 1'b0;
    assign IMU_SCLK = 1'b0;
    assign IMU_SDI = 1'b0;
    assign IMU_CS_AG = 1'b1;
    assign IMU_CS_M = 1'b1;
    assign IMU_DEN_AG = 1'b0;

     // mode
     wire [1:0] mode = SW[11:10];
     reg [7:0] mode_led;
     assign SS_ANODE = 4'b0111;
     assign SS_CATHODE = mode_led;
     always @ (mode)
     begin
         case (mode)
              2'b00: mode_led = 8'b11000000;
              2'b01: mode_led = 8'b11111001;
              2'b10: mode_led = 8'b10100100;
              2'b11: mode_led = 8'b10110000;
          endcase
    end

    // inputs    
    wire [3:0] a = SW[3:0];
    wire [3:0] b = SW[7:4];

    // add
    // the painful way to add two 4 bit numbers with fa and ha
    wire [4:0] c;
    add4_with_fa_ha adder(a, b, c);
     
    // the easy way to add two 4-bit numbers
    wire [4:0] d;
    assign d = a + b;
     
    // comparisons
    wire gt, lt, eq, ne;
    assign gt = a > b;
    assign lt = a < b;
    assign eq = a == b;
    assign ne = a != b;
     
    // count
    wire [2:0] count;
    count_ones_in_4_bits cnt(a, count);
    
    // outputs
    wire [9:0] out[3:0];
    assign out[0] = {d, c};
    assign out[1] = {6'b0, gt, lt, eq, ne};
    assign out[2] = {7'b0, count};
    assign out[3] = SW[9:0];
    assign LED = out[mode]; 
   
endmodule

// While this example includes these modules in the combo_logic.sv file,
// it is better practice to put each module in a separate file with
// a filename matching the module name

module count_ones_in_4_bits(
    input [3:0] in,
    output [2:0] out);
    
    assign out = in[0] + in[1] + in[2] + in[3];
endmodule

module add4_with_fa_ha(
    input [3:0] in1,
    input [3:0] in2,
    output [4:0] out);
     
    wire [2:0] carry;
    ha stage0(in1[0], in2[0], out[0], carry[0]);
    fa stage1(in1[1], in2[1], carry[0], out[1], carry[1]);
    fa stage2(in1[2], in2[2], carry[1], out[2], carry[2]);
    fa stage3(in1[3], in2[3], carry[2], out[3], out[4]);    
endmodule
     
module ha(
    input in1, 
    input in2,
    output out,
    output cout);
     
    // Structural Verilog
    assign out = in1 ^ in2;
    assign cout = in1 & in2;     
    // Behavioral Verilog
    // assign {cout, out} = in1 + in2;
endmodule

module fa(
    input in1, 
    input in2,
    input cin,
    output out,
    output cout);
   
    // Structural Verilog
    wire x;
    assign x = in1 ^ in2; // exactly one input is '1'    
    assign out = x ^ cin; // olld number of set {in1, in2, cin} is '1'
    assign cout = (in1 & in2) | (x & cin); // (exactly one input is '1' and cin '1' or in1 and in2 '1'
    // Behavioral Verilog
    // assign {cout, out} = in1 + in2 + cin;
endmodule
