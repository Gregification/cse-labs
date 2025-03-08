`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/01/2025 05:09:36 PM
// Design Name: 
// Module Name: lab4
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


module lab4(
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
    assign SS_ANODE = 4'b0000;
    assign SS_CATHODE = 8'b11111111;
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

    //---clocking, reset, & IO--------------------------------------------
    
    wire            clk = CLK100;
    reg     [31:0]  clk_ladder = 0;
        `define CLK_ILA clk
        `define CLK_TESTING clk_ladder[1]
        `define CLK_TEST_ITTER clk_ladder[1]
        `define CLK_IO clk_ladder[15]

    always_ff @ (posedge clk)
        clk_ladder <= clk_ladder + 1;

    reg     [3:0]   btns;
    reg     [11:0]  sws;

    always_ff @ (posedge clk)
        clk_ladder <= clk_ladder + 1;
    
    MSBuffer #(
        .WIDTH(4)
    ) (
        .clk(!`CLK_IO),
        .sig_in(PB[3:0]),
        .sig_out(btns[3:0])
    );
    
    MSBuffer #(
        .WIDTH(12)
    ) (
        .clk(!`CLK_IO),
        .sig_in(SW[11:0]),
        .sig_out(sws[11:0])
    );

    //---actual lab4 stuff--------------------------------------------------
    
    // test cases
    reg     [2:0][96:0] test_cases; // [entries][entry size]

    // inputs
    reg [31:2]  i_addr;     // i
    reg [31:0]  i_rdata;    // o
    reg [31:2]  d_addr;     // i
    reg         d_we;       // i
    reg [31:0]  d_wdata;    // i
    reg [3:0]   d_be;       // i

    //outputs
    reg [31:0]  d_rdata;    // o

    dual_port_ram _dual_port_ram (
         .clk(`CLK_TESTING),

         // instr port (ro)
         .i_addr(i_addr),
         .i_rdata(i_rdata),

         // data port (rw)
         .d_addr(d_addr),
         .d_we(d_we),
         .d_wdata(d_wdata),
         .d_be(d_be),
         .d_rdata(d_rdata)
    );


    ila_0 your_instance_name (
        .clk(`CLK_ILA), // input wire clk

        .probe0(i_addr),    // input wire [29:0]  probe0  
        .probe1(i_rdata),   // input wire [31:0]  probe1 
        .probe2(d_addr),    // input wire [29:0]  probe2 
        .probe3(d_rdata),   // input wire [31:0]  probe3 
        .probe4(d_wdata),   // input wire [31:0]  probe4 
        .probe5(d_be),      // input wire [3:0]  probe5 
        .probe6(d_we),      // input wire [0:0]  probe6
        .probe7(`CLK_TESTING) // input wire [31:0]  probe7
    );

    //---display values-----------------------------------------------------

    reg  [7:0] test_index;        // hex immideate display, the 16bits currently shown. also acts as test case index

    initial test_index = 0;

    assign LED[9:0] = sws[9:0];

    //------control---------------------------------------------------------

    always_ff @ (posedge(`CLK_TEST_ITTER)) begin
        test_index <= test_index + 1;
    end

    // run test cases
    // assign {i_addr, d_addr, d_we, d_be, d_wdata} = test_cases[text_index];
    // assign test_cases           = {
    //     //  {i_addr 30b     , d_addr 30b    , d_we 1b   , d_be 4b  , d_wdata 32b}
    //         {30'h0          , 32'h0         , 32'h0     , 32'h0    , 32'h0     },
    //         {30'h0          , 32'h0         , 32'h0     , 32'h0    , 32'h0     },
    // };
    
    // dump memory
    // assign i_addr   = {test_index, 2'b0};
    assign i_addr   = test_index;
    assign d_addr   = {test_index, 2'b0};
    assign d_be     = 4'b1111;
    assign d_wdata  = 0;
    assign d_we     = 0;

endmodule