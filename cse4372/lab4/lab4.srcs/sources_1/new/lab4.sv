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
        `define CLK_TESTING clk
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
    
    reg trigger;
    reg  [27:0] test_index;        // hex immideate display, the 16bits currently shown. also acts as test case index

    // test cases
    reg     [9:0][67:0] test_cases; // [entries][entry size]

    // dual port ram
    reg [31:2]  i_addr;     // i
    reg [31:0]  i_rdata;    // o
    reg [31:2]  d_addr;     // i
    reg         d_we;       // i
    reg [31:0]  d_wdata;    // i
    reg [3:0]   d_be;       // i
    reg [31:0]  d_rdata;    // o

    // mem shifter , inputs are buffered to simulate pipeline
    reg [31:0]  ms_d_rdata_shifted;
    reg         ms_unsigned;
    reg [3:0]   ms_d_be;

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

    mem_shifter_post _mem_shifter_post (
        .clk(`CLK_TESTING),

        .d_rdata(d_rdata),
        .d_rdata_shifted(ms_d_rdata_shifted),

        .isUnsigned(ms_unsigned),
        .d_be(ms_d_be)
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
        .probe7(`CLK_TESTING),  // input wire [0:0]  probe7
        .probe8(trigger),   // input wire [0:0]  probe8
        .probe9(test_index),// input wire [15:0]  probe9
        .probe10(ms_d_rdata_shifted),   // input wire [31:0]  probe10
        .probe11(ms_d_be),  // input wire [3:0]  probe11 
	    .probe12(d_be)      // input wire [3:0]  probe12
    );

    //---display values-----------------------------------------------------

    initial test_index = 0;

    assign LED[9:0] = sws[9:0];

    //------control---------------------------------------------------------

    assign trigger = btns[3];

    reg [1:0] test_width;

    always_ff @ (posedge(`CLK_TESTING)) begin
        if(! trigger)  begin
            i_addr  <= 0;
            d_addr  <= 0;
            d_be    <= 0;
            d_wdata <= 0;
            d_we    <= 0;
            test_index <= 0;
        end else begin
            // dump memory from i_addr
            if(sws[0]) begin
                test_index <= test_index + 1;

                i_addr   <= test_index;
                d_addr   <= 0;
                d_be     <= 4'b1111;
                d_wdata  <= 0;
                d_we     <= 0;
            end 

            // dump memory from d_addr
            else if(sws[1]) begin
                test_index <= test_index + 1;

                i_addr   <= 0;
                d_addr   <= test_index;
                d_be     <= test_index[3:0];
                d_wdata  <= 0;
                d_we     <= 0;

                if(sws[11])
                    ms_unsigned <= !test_index[0];
                else
                    ms_unsigned <= test_index[0];
                ms_d_be  <= test_index[3:0];
            end 

            // write then read
            else if(sws[2]) begin
                test_index <= test_index + 1;

                i_addr   <= test_index-1;
                d_addr   <= test_index;
                d_be     <= 4'b1111;
                d_wdata  <= test_index + 5;
                d_we     <= 1;
            end

            // run test cases 
            else if(sws[3]) begin
                test_index <= (test_index + 1) % $size(test_cases);

                i_addr[31:2] <= $size(test_cases) - 1 - test_index;

                // 'pipeline' values to memory shifter
                if(test_index > 0)
                    ms_unsigned <= test_cases[test_index - 1][0];
                else
                    ms_unsigned <= test_cases[$size(test_cases) - 1][0];
                ms_d_be <= d_be;

                // new test case
                {d_wdata, d_addr, test_width, d_we} = test_cases[test_index][67:1];

                case (test_width)
                    2'h0: d_be      <= 4'b0001;
                    2'h1: d_be      <= 4'b0011;
                    2'h2: d_be      <= 4'b1111;
                    default: d_be   <= 4'b1111;
                endcase

            end
        end
    end

    // for lab document step 9
    assign test_cases           = {
        // test write byte unsigned
        //  {d_wdata 32b    , d_addr 32b        , width 2b  , write 1b , unsign 1b  }
            {32'b1010       , {30'd1    , 2'b0} , 2'h0      , 1'b1      , 1'b1      }, // w byte 0b1010 as unsigned
            {32'b1010       , {30'd1    , 2'b0} , 2'h0      , 1'b0      , 1'b0      }, // r byte as signed
            {32'b1010       , {30'd1    , 2'b0} , 2'h0      , 1'b0      , 1'b1      }, // r byte as unsigned
            {32'b1010       , {30'd1    , 2'b0} , 2'h1      , 1'b0      , 1'b0      }, // r half word as signed
            {32'b1010       , {30'd1    , 2'b0} , 2'h1      , 1'b0      , 1'b1      }, // r half word as unsigned
            {32'b1010       , {30'd1    , 2'b0} , 2'h2      , 1'b0      , 1'b0      }, // r word as signed
            {32'b1010       , {30'd1    , 2'b0} , 2'h2      , 1'b0      , 1'b1      }, // r word as unsigned
        
        // test write byte signed
        //  {d_wdata 32b    , d_addr 32b        , width 2b  , write 1b , unsign 1b  }
            {32'b1010       , {30'd2    , 2'b0} , 2'h0      , 1'b1      , 1'b0      }, // w byte 0b1010 as signed
            {32'b1010       , {30'd2    , 2'b0} , 2'h0      , 1'b0      , 1'b0      }, // r byte as signed
            {32'b1010       , {30'd2    , 2'b0} , 2'h0      , 1'b0      , 1'b1      }, // r byte as unsigned
            {32'b1010       , {30'd2    , 2'b0} , 2'h1      , 1'b0      , 1'b0      }, // r half word as signed
            {32'b1010       , {30'd2    , 2'b0} , 2'h1      , 1'b0      , 1'b1      }, // r half word as unsigned
            {32'b1010       , {30'd2    , 2'b0} , 2'h2      , 1'b0      , 1'b0      }, // r word as signed
            {32'b1010       , {30'd2    , 2'b0} , 2'h2      , 1'b0      , 1'b1      }, // r word as unsigned


        // test write half word unsigned
        //  {d_wdata 32b    , d_addr 32b        , width 2b  , write 1b , unsign 1b  }
            {32'h9A         , {30'd3    , 2'b0} , 2'h1      , 1'b1      , 1'b1      }, // w half word 0b1001_1010 as unsigned
            {32'h9A         , {30'd3    , 2'b0} , 2'h0      , 1'b0      , 1'b0      }, // r byte as signed
            {32'h9A         , {30'd3    , 2'b0} , 2'h0      , 1'b0      , 1'b1      }, // r byte as unsigned
            {32'h9A         , {30'd3    , 2'b0} , 2'h1      , 1'b0      , 1'b0      }, // r half word as signed
            {32'h9A         , {30'd3    , 2'b0} , 2'h1      , 1'b0      , 1'b1      }, // r half word as unsigned
            {32'h9A         , {30'd3    , 2'b0} , 2'h2      , 1'b0      , 1'b0      }, // r word as signed
            {32'h9A         , {30'd3    , 2'b0} , 2'h2      , 1'b0      , 1'b1      }, // r word as unsigned

        // test write half word signed
        //  {d_wdata 32b    , d_addr 32b        , width 2b  , write 1b , unsign 1b  }
            {32'h9A         , {30'd4    , 2'b0} , 2'h1      , 1'b1      , 1'b0      }, // w half word 0b1001_1010 as signed
            {32'h9A         , {30'd4    , 2'b0} , 2'h0      , 1'b0      , 1'b0      }, // r byte as signed
            {32'h9A         , {30'd4    , 2'b0} , 2'h0      , 1'b0      , 1'b1      }, // r byte as unsigned
            {32'h9A         , {30'd4    , 2'b0} , 2'h1      , 1'b0      , 1'b0      }, // r half word as signed
            {32'h9A         , {30'd4    , 2'b0} , 2'h1      , 1'b0      , 1'b1      }, // r half word as unsigned
            {32'h9A         , {30'd4    , 2'b0} , 2'h2      , 1'b0      , 1'b0      }, // r word as signed
            {32'h9A         , {30'd4    , 2'b0} , 2'h2      , 1'b0      , 1'b1      }  // r word as unsigned
    };

endmodule