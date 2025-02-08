`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/02/2025 08:36:02 AM
// Design Name: 
// Module Name: top
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

/**
 * baselined off of the seq_logic.sv example
*/


module top(
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
    assign LED = 10'b0000000000;
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
    
    wire clk = CLK100;
    reg [31:0] clk_ladder = 0;


    // clock ladder
    always_ff @ (posedge(clk)) begin
        clk_ladder <= clk_ladder + 1;
    end

    // internal nets
    wire reset;
    reg [11:0]  sw_buffer;
    reg [159:0] test_case;
    reg [8:0]   test_idx;
    reg [31:0]  test_iw_in;
    reg [31:0]  test_pc_in;
    reg [31:0]  test_rs1_data;
    reg [31:0]  test_rs2_data;
    reg [31:0]  test_alu_out;

    reg [3:0][159:0] test_cases = {
    //  {32'h pc_in     ,32'b iw_in     , 32'b rs1_d_in , 32'b rs2_d_in , 32'b expcted alu_out},
        {32'h00000000   , 32'h00000001  , 32'h00000002  , 32'h00000003  , 32'h00000004},
        {32'h0000000a   , 32'h0000000b  , 32'h0000000c  , 32'h0000000d  , 32'h0000000e},
        {32'h00000014   , 32'h00000015  , 32'h00000016  , 32'h00000017  , 32'h00000018}
    };

    assign test_pc_in   = test_case[31:0];
    assign test_iw_in   = test_case[63:32];
    assign test_rs1_data= test_case[95:64];
    assign test_rs2_data= test_case[127:96];
    assign test_expected_alu_out = test_case[159:128];
    assign test_case    = test_cases[test_case_idx];
    assign test_case_idx= sw_buffer[7:0];

    LatchBuffer _reset_buffer(
        .clk(clk),
        .sig_in(PB[0]),
        .sig_out(reset)
    );
    LatchBuffer #(
        .WIDTH(12)
    )_sw_buffer(
        .clk(clk),
        .sig_in(SW[11:0]),
        .sig_out(sw_buffer[11:0])
    );

    rv32_ex_top test_target(
        .clk(clk_slow),
        .reset(reset),

        .pc_in(test_pc_in),
        .iw_in(test_iw_in),
        .rs1_data_in(test_rs1_data),
        .rs2_data_in(test_rs2_data),

        .alu_out(test_alu_out)
    );

    // hex display
    reg [15:0] hex_display;
    reg [31:0] hex_targ;
    wire [2:0] disp_idx;

    assign disp_idx = sw_buffer[10:8];
    assign hex_display = sw_buffer[11] ? hex_targ[31:16] : hex_targ[15:0];

    always_comb begin

        case(disp_idx)
            3'b000: hex_targ <= test_pc_in;
            3'b001: hex_targ <= test_iw_in;
            3'b010: hex_targ <= test_rs1_data;
            3'b011: hex_targ <= test_rs2_data;
            3'b100: hex_targ <= test_alu_out;
            3'b101: hex_targ <= test_expected_alu_out;
            default: hex_targ <= 32'hAAAAAAAA;
        endcase
    end

    reg [3:0] bcd_out [3:0];
    bin2bcd #(
        .WIDTH(16),
        .DIGITS(4)
    )_bcd (
        .bin(hex_display),
        .sign(0),
        .bcd(bcd_out)
    );

    reg [3:0] anode_b;
    reg [7:0] cathode_b;
    reg [3:0] bcd;
    assign SS_ANODE = anode_b;
    assign SS_CATHODE = cathode_b;
    always_ff @ (posedge(clk_ladder[18])) begin
        case(anode_b)
            4'b1110 : begin 
                anode_b <= 4'b1101;
                bcd <= bcd_out[1];
            end
            4'b1101 : begin 
                anode_b <= 4'b1011;
                bcd <= bcd_out[2];
            end
            4'b1011 : begin 
                anode_b <= 4'b0111;
                bcd <= bcd_out[3];
            end
            4'b0111 : begin 
                anode_b <= 4'b1110;
                bcd <= bcd_out[0];
            end
            default : begin 
                anode_b <= 4'b1110;
                bcd <= bcd_out[0];
            end
        endcase
        case(bcd)
            4'b0000 : cathode_b <= 8'b11000000;
            4'b0001 : cathode_b <= 8'b11111001;
            4'b0010 : cathode_b <= 8'b10100100;
            4'b0011 : cathode_b <= 8'b10110000;
            4'b0100 : cathode_b <= 8'b10011001;
            4'b0101 : cathode_b <= 8'b10010010;
            4'b0110 : cathode_b <= 8'b10000010;
            4'b0111 : cathode_b <= 8'b11111000;
            4'b1000 : cathode_b <= 8'b10000000;
            4'b1001 : cathode_b <= 8'b10011000;
            4'b1010 : cathode_b <= 8'b10001000;
            4'b1011 : cathode_b <= 8'b10000011;
            4'b1100 : cathode_b <= 8'b11000110;
            4'b1101 : cathode_b <= 8'b10100001;
            4'b1110 : cathode_b <= 8'b10000110;
            4'b1111 : cathode_b <= 8'b10001110;
            default : cathode_b <= 8'b11000010;
        endcase
    end

endmodule
