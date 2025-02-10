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
    
    //---clocking------------------------------------------------------------------

    wire clk = CLK100;
    reg [31:0] clk_ladder = 0;
        `define CLK_EX clk_ladder[10]           // ex clock should be faster than all others
        `define CLK_TESTING clk_ladder[17]
        `define CLK_HEX_DISPLAY clk_ladder[10]

    always_ff @ (posedge(clk))
        clk_ladder <= clk_ladder + 1;
        
    //---main testing logic--------------------------------------------------------

    // internal nets
    wire reset;
    reg [11:0]  sw_buffer;

    // reg     [159:0] test_cases[]; // synthesis error "dynamic range is not supported" ...
    reg     [88:0][159:0] test_cases;
    reg     [7:0]   test_idx;
    wire    [31:0]  test_iw_in;
    wire    [31:0]  test_pc_in;
    wire    [31:0]  test_rs1_data;
    wire    [31:0]  test_rs2_data;
    wire    [31:0]  test_alu_out;
    wire    [31:0]  test_expected_alu_out;

    assign LED = (test_alu_out != test_expected_alu_out) ? test_idx : 0;
    assign {test_pc_in, test_iw_in, test_rs1_data, test_rs2_data, test_expected_alu_out} = test_cases[test_idx];

    // iw macros. unused address bits are filled with 1's
    `define IW_ADD      (32'b00000001111111111000111110110011)
    `define IW_ADDI_M   (32'b00000000000011111000111110010011)
    `define IW_SUB      (32'b01000001111111111000111110110011)
    `define IW_AND      (32'b00000001111111111111111110110011)
    `define IW_ANDI_M   (32'b00000000000011111111111110010011)
    `define IW_OR       (32'b00000001111111111110111110110011)
    `define IW_ORI_M    (32'b00000000000011111110111110010011)
    `define IW_XOR      (32'b00000001111111111100111110110011)
    `define IW_XORI_M   (32'b00000000000011111100111110010011)
    `define IW_SLL      (32'b00000001111111111001111110110011)
    `define IW_SLLI_M   (32'b00000000000000000001111110010011)
    `define IW_SRL      (32'b00000001111111111101111110110011)
    `define IW_SRLI_M   (32'b00000000000011111101111110010011)
    `define IW_SRA      (32'b01000001111111111101111110110011)
    `define IW_SRAI_M   (32'b01000000000011111101111110010011)
    `define IW_JALR_M   (32'b00000000000011111000111111100111)
    `define IW_JAL_M    (32'b00000000000000000000111111101111)
    `define IW_LUI_M    (32'b00000000000000000000111110110111)
    `define IW_AIPIC_M  (32'b)
    
    /** note
     *      - indexes are numbered bottom to top
     */
     /**
            3'b000: test_pc_in
            3'b001: test_iw_in
            3'b010: test_rs1_data
            3'b011: test_rs2_data
            3'b100: test_expected_alu_out
            3'b101: test_alu_out
            3'b110: test_idx
            3'b111: test_pc_in (incrementing idx)
    */
    assign test_cases           = {
        //  {pc         , iw mask     | immediate value     , rs1_d     , rs2_d ,  expcted alu_out},
            { 32'h0095  ,  `IW_JALR_M | ( 12'h0   << 20)    , -32'h1    , 32'h0 ,  32'h0     },
            { 32'h0094  ,  `IW_JALR_M | (-12'hF   << 20)    ,  32'h0    , 32'h1 , -32'hE     },
            { 32'h0093  ,  `IW_JALR_M | (-12'hE   << 20)    , -32'h1    , 32'h2 , -32'hE     },
            { 32'h0092  ,  `IW_JALR_M | ( 12'h5D  << 20)    , -32'h0    , 32'h3 ,  32'h5C    },
            { 32'h0091  ,  `IW_JALR_M | ( 12'hEA  << 20)    ,  32'h3    , 32'h4 ,  32'hEC    },
            { 32'h0089  ,  `IW_JALR_M | ( 12'hFF  << 20)    ,  32'h5    , 32'h5 ,  32'h104   },
            { 32'h0088  ,  `IW_JALR_M | ( 12'h8   << 20)    ,  32'hF    , 32'h6 ,  32'h16     },

        //  {pc         , iw mask     | immediate value                     , rs1_d , rs2_d , expcted alu_out},
            { 32'h0087  ,  `IW_JAL_M  | (20'b01010100011101010110 << 12)    , 32'hD , 32'hF , 32'h56DCD }, // i = 20'b00101011011010100011 = 20'h2B6A3
            { 32'h0086  ,  `IW_JAL_M  | (20'b1 << 12)                       , 32'h0 , 32'hF , 32'h1086  }, // i = 1<<12

        //  {pc         , iw mask     | shamt               , rs1_d     , rs2_d , expcted alu_out},
            { 32'h0085  ,  `IW_SRAI_M | ( 5'h0    << 20)    , -32'h100  , 32'hF , -32'h100  },
            { 32'h0084  ,  `IW_SRAI_M | ( 5'hFF   << 20)    , -32'h1    , 32'h0 , -32'h1    },
            { 32'h0083  ,  `IW_SRAI_M | ( 5'h2    << 20)    ,  32'h100  , 32'h0 ,  32'h40   },
            { 32'h0082  ,  `IW_SRAI_M | ( 5'h3    << 20)    ,  32'h100  , 32'hF ,  32'h20   },
            { 32'h0081  ,  `IW_SRAI_M | ( 5'h4    << 20)    ,  32'h100  , 32'hF ,  32'h10   },

        //  {pc         , iw mask       , rs1_d         , rs2_d     , expcted alu_out},
            { 32'h0080  ,  `IW_SRA      , -32'h1        ,  32'hFF   ,  -32'h1       },
            { 32'h0079  ,  `IW_SRA      , -32'h2        ,  32'h1    ,  -32'h1       },
            { 32'h0078  ,  `IW_SRA      ,  32'h1000     ,  32'h02   ,  32'h400      },
            { 32'h0077  ,  `IW_SRA      ,  32'h2000     ,  32'h00   ,  32'h2000     },
            { 32'h0076  ,  `IW_SRA      ,  32'h4000     ,  32'h00   ,  32'h4000     },

        //  {pc         , iw mask     | shamt               , rs1_d     , rs2_d , expcted alu_out},
            { 32'h0075  ,  `IW_SRLI_M | ( 5'h0    << 20)    ,  32'h100  , 32'hF , 32'h100   },
            { 32'h0074  ,  `IW_SRLI_M | ( 5'h1    << 20)    ,  32'h100  , 32'hF , 32'h80    },
            { 32'h0073  ,  `IW_SRLI_M | ( 5'h2    << 20)    ,  32'h100  , 32'hF , 32'h40    },
            { 32'h0072  ,  `IW_SRLI_M | ( 5'h3    << 20)    ,  32'h100  , 32'h0 , 32'h20    },
            { 32'h0071  ,  `IW_SRLI_M | ( 5'h4    << 20)    ,  32'h100  , 32'h0 , 32'h10    },

        //  {pc         , iw mask       , rs1_d         , rs2_d     , expcted alu_out},
            { 32'h0070  ,  `IW_SRL      , -32'h1        ,  32'hFF   ,  32'h1        },
            { 32'h0069  ,  `IW_SRL      ,  32'h1000     ,  32'h02   ,  32'h400      },
            { 32'h0068  ,  `IW_SRL      ,  32'h2000     ,  32'h00   ,  32'h2000     },
            { 32'h0067  ,  `IW_SRL      ,  32'hF00000   ,  32'hF3   ,  32'h1E       },
            { 32'h0066  ,  `IW_SRL      ,  32'hF5       ,  32'hF0   ,  32'h0        },

        //  {pc         , iw mask     | shamt               , rs1_d     , rs2_d , expcted alu_out},
            { 32'h0065  ,  `IW_SLLI_M | ( 5'h0    << 20)    ,  32'h10 , 32'h0 , 32'h00000010 },
            { 32'h0064  ,  `IW_SLLI_M | ( 5'h1    << 20)    ,  32'h10 , 32'hF , 32'h00000020 },
            { 32'h0063  ,  `IW_SLLI_M | ( 5'h2    << 20)    ,  32'h10 , 32'hF , 32'h00000040 },
            { 32'h0062  ,  `IW_SLLI_M | ( 5'hF0   << 20)    ,  32'h10 , 32'hF , 32'h00100000 },
            { 32'h0061  ,  `IW_SLLI_M | ( 5'hF4   << 20)    ,  32'h10 , 32'h0 , 32'h01000000 },

        //  {pc         , iw mask       , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0060  ,  `IW_SLL      ,  32'h0    ,  32'hF5   ,  32'h00000000 },
            { 32'h0059  ,  `IW_SLL      ,  32'h1    ,  32'h02   ,  32'h00000004 },
            { 32'h0058  ,  `IW_SLL      ,  32'h20   ,  32'h00   ,  32'h00000020 },
            { 32'h0057  ,  `IW_SLL      ,  32'hF0   ,  32'hF3   ,  32'h07800000 },
            { 32'h0056  ,  `IW_SLL      ,  32'hF4   ,  32'hF5   ,  32'h1E800000 },

        //  {pc         , iw mask     | immideate value     , rs1_d     , rs2_d ,  expcted alu_out},
            { 32'h0055  ,  `IW_XORI_M | ( 12'h0   << 20)    ,  32'h0    , 32'h0 ,  32'h0     },
            { 32'h0054  ,  `IW_XORI_M | ( 12'h1   << 20)    ,  32'h0    , 32'hF ,  32'h1     },
            { 32'h0053  ,  `IW_XORI_M | ( 12'h0   << 20)    ,  32'h1    , 32'hE ,  32'h1     },
            { 32'h0052  ,  `IW_XORI_M | ( 12'h1   << 20)    ,  32'h1    , 32'hD ,  32'h0     },
            { 32'h0051  ,  `IW_XORI_M | ( 12'hE   << 20)    ,  32'hF    , 32'h0 ,  32'h1     },

        //  {pc         , iw mask     | immideate value     , rs1_d     , rs2_d ,  expcted alu_out},
            { 32'h0050  ,  `IW_ORI_M  | ( 12'h0   << 20)    ,  32'h0    , 32'h0 ,  32'h0     },
            { 32'h0049  ,  `IW_ORI_M  | ( 12'h1   << 20)    ,  32'h0    , 32'hC ,  32'h1     },
            { 32'h0048  ,  `IW_ORI_M  | ( 12'h0   << 20)    ,  32'h1    , 32'hB ,  32'h1     },
            { 32'h0047  ,  `IW_ORI_M  | ( 12'h1   << 20)    ,  32'h1    , 32'hA ,  32'h1     },
            { 32'h0046  ,  `IW_ORI_M  | ( 12'hE   << 20)    ,  32'hF    , 32'h0 ,  32'hF     },

        //  {pc         , iw mask     | immideate value     , rs1_d     , rs2_d ,  expcted alu_out},
            { 32'h0045  ,  `IW_ANDI_M | ( 12'h0   << 20)    ,  32'h0    , 32'hE ,  32'h0     },
            { 32'h0044  ,  `IW_ANDI_M | ( 12'h1   << 20)    ,  32'h0    , 32'h0 ,  32'h0     },                        
            { 32'h0043  ,  `IW_ANDI_M | ( 12'h0   << 20)    ,  32'h1    , 32'hF ,  32'h0     },
            { 32'h0042  ,  `IW_ANDI_M | ( 12'h1   << 20)    ,  32'h1    , 32'h0 ,  32'h1     },
            { 32'h0041  ,  `IW_ANDI_M | ( 12'hE   << 20)    ,  32'hF    , 32'h0 ,  32'hE     },

        //  {pc         , iw            , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0040  ,  `IW_AND      ,  32'h0    ,  32'h0    ,  32'h0   },
            { 32'h0039  ,  `IW_AND      ,  32'h1    ,  32'h0    ,  32'h0   },
            { 32'h0038  ,  `IW_AND      ,  32'h0    ,  32'h1    ,  32'h0   },
            { 32'h0037  ,  `IW_AND      ,  32'h1    ,  32'h1    ,  32'h1   },
            { 32'h0036  ,  `IW_AND      ,  32'hE    ,  32'hF    ,  32'hE   },

        //  {pc         , iw            , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0035  ,  `IW_OR       ,  32'h0    ,  32'h0    ,  32'h0   },
            { 32'h0034  ,  `IW_OR       ,  32'h1    ,  32'h0    ,  32'h1   },
            { 32'h0033  ,  `IW_OR       ,  32'h0    ,  32'h1    ,  32'h1   },
            { 32'h0032  ,  `IW_OR       ,  32'h1    ,  32'h1    ,  32'h1   },
            { 32'h0031  ,  `IW_OR       ,  32'hE    ,  32'hF    ,  32'hF   },

        //  {pc         , iw            , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0030  ,  `IW_XOR      ,  32'h0    ,  32'h0    ,  32'h0   },
            { 32'h0029  ,  `IW_XOR      ,  32'h1    ,  32'h0    ,  32'h1   },
            { 32'h0028  ,  `IW_XOR      ,  32'h0    ,  32'h1    ,  32'h1   },
            { 32'h0027  ,  `IW_XOR      ,  32'h1    ,  32'h1    ,  32'h0   },
            { 32'h0026  ,  `IW_XOR      ,  32'hE    ,  32'hF    ,  32'h1   },

        //  {pc         , iw            , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0025  ,  `IW_ADD      ,  32'h1    ,  32'h2    ,  32'h3   },
            { 32'h0024  ,  `IW_ADD      ,  32'h2    ,  32'h1    ,  32'h3   },
            { 32'h0023  ,  `IW_ADD      , -32'd1    ,  32'd1    ,  32'd0   },
            { 32'h0022  ,  `IW_ADD      , -32'd1    ,  32'h1    ,  32'h0   },
            { 32'h0021  ,  `IW_ADD      , -32'd1    , -32'd1    , -32'd2   },
            { 32'h0020  ,  `IW_ADD      ,  32'd23   , -32'd654  , -32'd631 },
            { 32'h0019  ,  `IW_ADD      , -32'd1    ,  32'd0    , -32'd1   },
            { 32'h0018  ,  `IW_ADD      ,  32'd0    , -32'd1    , -32'd1   },
            { 32'h0017  ,  `IW_ADD      ,  32'd0    ,  32'd0    ,  32'd0   },

        //  {pc         , iw            , rs1_d     , rs2_d     , expcted alu_out},
            { 32'h0016  ,  `IW_SUB      ,  32'h1    ,  32'h2    , -32'h1   },
            { 32'h0015  ,  `IW_SUB      ,  32'h2    ,  32'h1    ,  32'h1   },
            { 32'h0014  ,  `IW_SUB      , -32'd1    ,  32'd1    , -32'd2   },
            { 32'h0013  ,  `IW_SUB      , -32'd1    , -32'd1    ,  32'd0   },
            { 32'h0012  ,  `IW_SUB      ,  32'd1    , -32'd1    ,  32'd2   },
            { 32'h0011  ,  `IW_SUB      ,  32'd23   , -32'd654  ,  32'd677 },
            { 32'h0010  ,  `IW_SUB      , -32'd1    ,  32'd0    , -32'd1   },
            { 32'h0009  ,  `IW_SUB      ,  32'd0    , -32'd1    ,  32'd1   },
            { 32'h0008  ,  `IW_SUB      ,  32'd0    ,  32'd0    ,  32'd0   },

        //  {pc         , iw mask     | immideate value     , rs1_d     , rs2_d ,  expcted alu_out},
            { 32'h0007  ,  `IW_ADDI_M | ( 12'h0   << 20)    ,  32'h0    , 32'h1 ,  32'h0     },
            { 32'h0006  ,  `IW_ADDI_M | ( 12'h1   << 20)    ,  32'h0    , 32'h2 ,  32'h1     },
            { 32'h0005  ,  `IW_ADDI_M | ( 12'h0   << 20)    , -32'h35   , 32'h3 , -32'h35    },
            { 32'h0004  ,  `IW_ADDI_M | ( 12'h2   << 20)    ,  32'h0    , 32'h0 ,  32'h2     },
            { 32'h0003  ,  `IW_ADDI_M | (-12'h5   << 20)    ,  32'h0    , 32'h5 , -32'h5     },
            { 32'h0002  ,  `IW_ADDI_M | ( 12'h1   << 20)    ,  32'h1    , 32'h6 ,  32'h2     },
            { 32'h0001  ,  `IW_ADDI_M | (-12'h1   << 20)    , -32'h3    , 32'h0 , -32'h4     }
        };

    LatchBuffer _reset_buffer(
        .clk(!`CLK_EX),
        .sig_in(PB[0]),
        .sig_out(reset)
    );
    LatchBuffer #(
        .WIDTH(12)
    )_sw_buffer(
        .clk(!`CLK_EX),
        .sig_in(SW[11:0]),
        .sig_out(sw_buffer[11:0])
    );

    rv32_ex_top test_target(
        .clk(`CLK_EX),
        .reset(reset),

        .pc_in(test_pc_in),
        .iw_in(test_iw_in),
        .rs1_data_in(test_rs1_data),
        .rs2_data_in(test_rs2_data),

        .alu_out(test_alu_out)
    );

    //---hex display & test auto increment-----------------------------------------

    reg  [15:0] hex_display;
    reg  [31:0] hex_targ;
    wire [2:0] disp_idx;
    reg  [7:0] increment;
    // initial increment = 0;

    assign test_idx[7:0] = (disp_idx == 3'b111) ? increment[7:0] : sw_buffer[7:0];
    assign disp_idx = sw_buffer[10:8];
    assign hex_display = sw_buffer[11] ? hex_targ[31:16] : hex_targ[15:0];

    // what to display
    always_comb begin
        case(disp_idx)
            3'b000: hex_targ <= test_pc_in;
            3'b001: hex_targ <= test_iw_in;
            3'b010: hex_targ <= test_rs1_data;
            3'b011: hex_targ <= test_rs2_data;
            3'b100: hex_targ <= test_expected_alu_out;
            3'b101: hex_targ <= test_alu_out;
            3'b110: hex_targ <= test_idx;
            3'b111: hex_targ <= test_pc_in;
            default: hex_targ <= 32'hAAAAAAAA;
        endcase
    end

    // auto testing increment
    always_ff @ (posedge(`CLK_TESTING)) begin
        if(reset)
            increment <= 0;
        if((disp_idx == 3'b111) && (test_expected_alu_out == test_alu_out))
            increment <= increment + 1;

    end

    //
    // hex display driver
    //

    reg [3:0] anode_b;
    reg [7:0] cathode_b;
    reg [3:0] bin;
    initial begin
        anode_b <= 4'b1110;
        cathode_b <= 8'b11000000;
        bin <= 4'b0000;
    end

    assign SS_ANODE = anode_b;
    assign SS_CATHODE = cathode_b;

    always_ff @ (posedge(`CLK_HEX_DISPLAY)) begin
        case(anode_b)
            4'b0111 : begin 
                anode_b <= 4'b1110;
                bin <= hex_display[3:0];
            end
            4'b1110 : begin 
                anode_b <= 4'b1101;
                bin <= hex_display[7:4];
            end
            4'b1101 : begin 
                anode_b <= 4'b1011;
                bin <= hex_display[11:8];
            end
            4'b1011 : begin 
                anode_b <= 4'b0111;
                bin <= hex_display[15:12];
            end
            default : begin 
                anode_b <= 4'b1110;
                bin <= hex_display[3:0];
            end
        endcase
    end
    always_comb
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

endmodule
