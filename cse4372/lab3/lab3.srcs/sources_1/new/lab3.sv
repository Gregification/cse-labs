`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/18/2025 09:09:59 PM
// Design Name: 
// Module Name: lab3
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


module lab3(
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
        `define CLK_TESTING clk_ladder[17]
        `define CLK_HEX_DISPLAY clk_ladder[12]
    wire            reset;
    reg     [3:0]   btns;
    reg     [11:0]  sws;

    assign reset = btns[0];
    assign LED[9:0] = sws[9:0];

    always_ff @ (posedge clk)
        clk_ladder <= clk_ladder + 1;
    
    SynchBuffer #(
        .WIDTH(4)
    ) _btn_buffer (
        .clk(!`CLK_IO),
        .sig_in(PB[3:0]),
        .sig_out(btns[3:0])
    );
    
    SynchBuffer #(
        .WIDTH(12)
    ) _sw_buffer (
        .clk(!`CLK_IO),
        .sig_in(SW[11:0]),
        .sig_out(sws[11:0])
    );

    //---actual lab3 stuff--------------------------------------------------
    
    //inputs
    reg [4:0]   rs1_reg;
    reg [4:0]   rs2_reg;
    reg         wb_enable;
    reg [4:0]   wb_reg;
    reg [31:0]  wb_data;

    // outputs
    reg [31:0]  rs1_data;
    reg [31:0]  rs2_data;

    rv32i_regs _regs(
        .clk(`CLK_TESTING),
        .reset(reset),
        .rs1_reg(rs1_reg),
        .rs2_reg(rs2_reg),
        .wb_enable(wb_enable),
        .wb_reg(wb_reg),
        .wb_data(wb_data),
        .rs1_data(rs1_data),
        .rs2_data(rs2_data)
    );

    //---display values-----------------------------------------------------

    reg [31:0]  hex_display;            // the 32bit value displayed
    wire        hex_toggle_show_upper;  // toggles showing upper/lower 16bits of the display value; 

    assign hex_toggle_show_upper = sws[11];

    //---testing/display logic------------------------------------------------------

    assign hex_display[31:0] = sws[10] ? wb_data : {rs1_data, rs2_data};

    reg             write_rs1_reg;
    reg             write_rs2_reg;
    reg             write_wb_reg;
    reg     [2:0]   wb_addr_to_write;
    wire    [31:0]  wb_data_to_addr;

    assign write_rs1_reg        = btns[3];
    assign write_rs2_reg        = btns[2];
    assign write_wb_reg         = btns[1];
    assign wb_enable            = sws[9];
    assign wb_data_to_addr [3:0]= sws[3:0];
    assign wb_addr_to_write     = sws[6:4];

    always_ff @ (posedge(`CLK_TESTING)) begin
        if(write_rs1_reg)
            rs1_reg <= sws[4:0];
        if(write_rs2_reg)
            rs2_reg <= sws[4:0];
        if(write_wb_reg) 
            wb_reg <= sws[4:0];

        if(sws[9]) begin
            //wb_reg[wb_addr_to_write*4:+4] <= wb_data_to_addr;
            wb_data = wb_data & ~((32'hF) << wb_addr_to_write*4);
            wb_data = wb_data | (wb_data_to_addr << wb_addr_to_write*4);
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
