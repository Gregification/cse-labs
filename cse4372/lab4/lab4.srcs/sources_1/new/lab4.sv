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

    //---clocking, reset, & IO--------------------------------------------
    
    wire            clk = CLK100;
    reg     [31:0]  clk_ladder = 0;
        `define CLK_IO clk_ladder[10]
        `define CLK_TESTING CLK100
        `define CLK_HEX_DISPLAY clk_ladder[12]
    wire            reset;
    reg     [3:0]   btns;
    reg     [11:0]  sws;

    assign reset = btns[0];
    // assign LED[9:0] = sws[9:0];

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
    
    // inputs
    reg [31:2]  i_addr;     // 0
    reg [31:0]  i_rdata;    // 1
    reg [31:2]  d_addr;     // 2
    reg         d_we;       // 3
    reg [31:0]  d_wdata;    // 4
    reg [3:0]   d_be;       // 5

    //outputs
    reg [31:0]  d_rdata;    // 6

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

    //---display values-----------------------------------------------------

    reg [31:0]  hex_display;            // the 32bit value displayed
    wire        hex_toggle_show_upper;  // toggles showing upper/lower 16bits of the display value; 

    assign hex_display = d_rdata;

    //---UI control---------------------------------------------------------

    wire [2:0]  op_sel  = sws[11:9];
    wire [2:0]  byte_sel= sws[8:6];     // byte index
    wire [3:0]  byte_val= sws[3:0];     // byte value
    wire [1:0]  led_disp= sws[5:4];

    assign hex_toggle_show_upper = |led_disp;

    reg [9:0]   LED_buffer;
    assign LED = LED_buffer;
    always case(led_disp)
        2'd0: LED_buffer[8:0]  = d_rdata[8:0];
        2'd1: LED_buffer[8:0]  = d_rdata[16:8];
        2'd2: LED_buffer[8:0]  = d_rdata[24:16];
        2'd3: LED_buffer[8:0]  = d_rdata[31:24];
        default: LED_buffer[8:0] = 0;
    endcase
    assign LED[9] = d_be;

    always_ff @ (posedge `CLK_TESTING) begin

        if(btns[3]) begin
            reg [31:0] targ;

            case(op_sel)
                default: targ   = i_addr;
                3'd0: targ  = i_addr;
                3'd1: targ  = i_rdata;
                3'd2: targ  = d_addr;
                3'd3: targ  = d_be;
                3'd4: targ  = d_rdata;
                3'd5: targ  = d_wdata;
            endcase

            case(byte_sel)
                default: targ[3:0]  = byte_val;
                3'd0: targ[3:0]     = byte_val;
                3'd1: targ[7:4]     = byte_val;
                3'd2: targ[11:8]    = byte_val;
                3'd3: targ[15:12]   = byte_val;
                3'd4: targ[19:16]   = byte_val;
                3'd5: targ[23:20]   = byte_val;
                3'd6: targ[27:24]   = byte_val;
                3'd7: targ[31:28]   = byte_val;
            endcase

            case(op_sel)
                default: i_addr = targ;
                3'd0: i_addr    = targ;
                3'd1: i_rdata   = targ;
                3'd2: d_addr    = targ;
                3'd3: d_be      = targ;
                3'd4: d_rdata   = targ;
                3'd5: d_wdata   = targ;
            endcase
        end
    end

    //---display drive logic------------------------------------------------

    reg  [15:0] hex_imm_display;        // hex immideate display, the 16bits currently shown
    reg [3:0] anode_b;
    reg [7:0] cathode_b;
    reg [3:0] bin;                      // value of the current refreshing hex digit

    assign SS_ANODE = anode_b;
    assign SS_CATHODE = cathode_b;
    assign hex_imm_display = hex_toggle_show_upper ? hex_display[31:16] : hex_display[15:0];

    initial begin
        anode_b <= 4'b1110;
        cathode_b <= 8'b11000000;
        bin <= 4'b0000;
    end

    always_ff @ (posedge(`CLK_HEX_DISPLAY)) begin
        case(anode_b)
            4'b0111 : begin 
                anode_b <= 4'b1110;
                bin <= hex_imm_display[3:0];
            end
            4'b1110 : begin 
                anode_b <= 4'b1101;
                bin <= hex_imm_display[7:4];
            end
            4'b1101 : begin 
                anode_b <= 4'b1011;
                bin <= hex_imm_display[11:8];
            end
            4'b1011 : begin 
                anode_b <= 4'b0111;
                bin <= hex_imm_display[15:12];
            end
            default : begin 
                anode_b <= 4'b1110;
                bin <= hex_imm_display[3:0];
            end
        endcase
    end

    always_comb begin
        case(bin)
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