`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/01/2025 01:22:58 PM
// Design Name: 
// Module Name: lab7
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


module lab8(
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
    
    reg     [3:0]   btns;
    reg     [11:0]  sws;

    reg             reset;

    wire            clk = CLK100;
    reg     [31:0]  clk_ladder = 0;
        `define CLK_ILA clk_ladder[0]
        `define CLK_PIPELINE clk_ladder[0]
        `define CLK_IO clk_ladder[10]

    always_ff @ (posedge clk)
        clk_ladder <= clk_ladder + 1;
    
    // metastable buffer : push buttons
    MSBuffer #(
        .WIDTH(4)
    ) (
        .clk(!`CLK_IO),
        .sig_in(PB[3:0]),
        .sig_out(btns[3:0])
    );
    
    // metastable buffer : switches
    MSBuffer #(
        .WIDTH(12)
    ) (
        .clk(!`CLK_IO),
        .sig_in(SW[11:0]),
        .sig_out(sws[11:0])
    );

    assign reset = btns[0];

    //---ILA----------------------------------------------------------------

    ila_0 your_instance_name (
        .clk(`CLK_ILA), // input wire clk

        // reset <1->0> used as trigger 
        .probe0(0), // input wire [0:0]  probe0  
        .probe1(reset), // input wire [0:0]  probe1 
        
        // if_top
        .probe2(_rv32_if_top.memif_data), // input wire [31:0]  probe2 

        // id_top
        .probe3(_rv32_id_top.pc_in), // input wire [31:0]  probe3 
        .probe4(_rv32_id_top.iw_in), // input wire [31:0]  probe4 
        .probe5(_rv32_id_top.regif_rs1_reg), // input wire [4:0]  probe5 
	    .probe6(_rv32_id_top.regif_rs2_reg), // input wire [4:0]  probe6 

        // ex_top
        .probe7(_rv32_ex_top.pc_in), // input wire [31:0]  probe7 
        .probe8(_rv32_ex_top.iw_in), // input wire [31:0]  probe8 
        .probe9(_rv32_ex_top.rs1_data_in), // input wire [31:0]  probe9 
        .probe10(_rv32_ex_top.rs2_data_in), // input wire [31:0]  probe10 
        .probe11(_rv32_ex_top.wb_reg_in), // input wire [4::0]  probe11 
        .probe12(_rv32_ex_top.wb_enable_in), // input wire [0:0]  probe12 

        // mem_top
        .probe13(_rv32_mem_top.pc_in), // input wire [31:0]  probe13 
        .probe14(_rv32_mem_top.iw_in), // input wire [31:0]  probe14 
        .probe15(_rv32_mem_top.alu_in), // input wire [31:0]  probe15 
        .probe16(_rv32_mem_top.wb_reg_in), // input wire [4:0]  probe16 
        .probe17(_rv32_mem_top.wb_enable_in), // input wire [0:0]  probe17 

        // wb_top
        .probe18(_rv32_wb_top.pc_in), // input wire [31:0]  probe18 
        .probe19(_rv32_wb_top.iw_in), // input wire [31:0]  probe19 
        .probe20(_rv32_wb_top.alu_in), // input wire [31:0]  probe20 
        .probe21(_rv32_wb_top.wb_reg_in), // input wire [4:0]  probe21 
        .probe22(_rv32_wb_top.wb_enable_in), // input wire [0:0]  probe22

        // wb output
        .probe23(_rv32_wb_top.regif_wb_enable), // input wire [0:0]  probe23
        .probe24(_rv32_wb_top.regif_wb_reg), // input wire [4:0]  probe24
        .probe25(_rv32_wb_top.regif_wb_data), // input wire [31:0]  probe25
        .probe26(_rv32_mem_top.io_we_in), // input wire [0:0]  probe26 ----------------
        .probe27(_rv32_if_top.reset), // input wire [0:0]  probe27

        // jump information
        .probe28(_rv32_id_top.jump_enable_out), // input wire [0:0]  probe28 
        .probe29(_rv32_id_top.jump_addr_out), // input wire [31:0]  probe29

        .probe30(_dual_port_ram.d_addr), // input wire [31:0]  probe30 
	    .probe31(_dual_port_ram.d_rdata), // input wire [31:0]  probe31 
        .probe32(_dual_port_ram.d_be), // input wire [3:0]  probe32 
        .probe33(_rv32_mem_top.memif_we_in), // input wire [0:0]  probe33 
        .probe34(_dual_port_ram.d_wdata), // input wire [31:0]  probe34
        .probe35(_rv32_wb_top.memif_rdata), // input wire [31:0]  probe35 
        .probe36(_rv32_wb_top.io_rdata), // input wire [31:0]  probe36
        .probe37(_dual_port_ram.d_we), // input wire [0:0]  probe37 
        .probe38(_rv32_wb_top.wb_from_alu_in), // input wire [31:0]  probe38
        .probe39(_rv32_mem_top.mem_addr_in), // input wire [31:0]  probe39 
        .probe40(0),//_rv32_mem_top.mem_addr_out), // input wire [31:0]  probe40 
        .probe41(_io_memory.addr), // input wire [31:0]  probe41
        .probe42(_io_memory.we), // input wire [31:0]  probe42 
        .probe43(_io_memory.wdata), // input wire [31:0]  probe43 
        .probe44(_io_memory.be), // input wire [3:0]  probe44 
        .probe45(_io_memory.led_out), // input wire [9:0]  probe45 
        .probe46(_io_memory.pb_in), // input wire [3:0]  probe46 
        .probe47(_io_memory.sw_in), // input wire [11:0]  probe47 
        .probe48(_io_memory.offset), // input wire [31:0]  probe48 
        .probe49(_io_memory.rdata) // input wire [31:0]  probe49
    );

    //---dual port memory---------------------------------------------------
    
    dual_port_ram _dual_port_ram (
        .clk(`CLK_PIPELINE),

        // Instruction port (RO)
        .i_addr(_rv32_if_top.memif_addr),
        // output reg [31:0] i_rdata,
        
        // Data port (RW)
        .d_addr(_rv32_mem_top.mem_addr_in),
        // output reg [31:0] d_rdata,
        .d_we(_rv32_mem_top.memif_we_out),
        .d_be(_rv32_mem_top.mem_be_in),
        .d_wdata(_rv32_mem_top.mem_wdata)
    );
    
    io_memory _io_memory (
        .clk(`CLK_PIPELINE),
        
        // access
        .addr(_rv32_mem_top.mem_addr_in),
        .we(_rv32_mem_top.io_we_out),
        .be(_rv32_mem_top.mem_be_in),
        .wdata(_rv32_mem_top.mem_wdata),
        // output reg [31:0] rdata,

        // // top routing outputs
        // .led_out(LED),

        // // top routing inputs
        .pb_in(btns),
        .sw_in(sws)
    );
    assign LED = _io_memory.led_out;

    //---registers----------------------------------------------------------

    rv32i_regs _rv32i_regs (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // inputs
        .rs1_reg(_rv32_id_top.regif_rs1_reg),
        .rs2_reg(_rv32_id_top.regif_rs2_reg),
        .wb_enable(_rv32_wb_top.regif_wb_enable),
        .wb_reg(_rv32_wb_top.regif_wb_reg),
        .wb_data(_rv32_wb_top.regif_wb_data)

        // outputs
        // output reg [31:0] rs1_data,
        // output reg [31:0] rs2_data
    );


    //---pipeline-----------------------------------------------------------
    // if -> id -> ex -> mem -> wb
    //      note : mem, wb, and regs kinda go everywhere though

    rv32_if_top _rv32_if_top (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // memory interface
        // output [31:2] memif_addr,
        .memif_data(_dual_port_ram.i_rdata), // in

        // to id
        // output reg [31:0] pc_out,
        // output [31:0] iw_out

        // from id
        .jump_enable_in(_rv32_id_top.jump_enable_out),
        .jump_addr_in(_rv32_id_top.jump_addr_out)
    );

    rv32_id_top _rv32_id_top (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // from if
        .pc_in(_rv32_if_top.pc_out),
        .iw_in(_rv32_if_top.iw_out),

        // register interface
        // output [4:0] regif_rs1_reg,
        // output [4:0] regif_rs2_reg,
        .regif_rs1_data_in(_rv32i_regs.rs1_data),
        .regif_rs2_data_in(_rv32i_regs.rs2_data),

        // // to ex
        // output reg [31:0] pc_out,
        // output reg [31:0] iw_out,
        // output reg [4:0] wb_reg_out,
        // output reg wb_enable_out,

        // df from ex
        .df_ex_wb_reg(_rv32_ex_top.wb_reg_in),
        .df_ex_wb_data(_rv32_ex_top.alu_raw),
        .df_ex_wb_enable(_rv32_ex_top.wb_enable_in),
        
        // df from mem
        .df_mem_wb_reg(_rv32_mem_top.wb_reg_in),
        .df_mem_wb_data(_rv32_mem_top.alu_in),
        .df_mem_wb_enable(_rv32_mem_top.wb_enable_in),

        // df from wb
        .df_wb_wb_reg(_rv32_wb_top.regif_wb_reg),
        .df_wb_wb_data(_rv32_wb_top.regif_wb_data),
        .df_wb_wb_enable(_rv32_wb_top.regif_wb_enable)

        // to id : regarding pc jumping
        // output reg jump_enable_out,
        // output reg [31:0] jump_addr_out

        // debugging to top
        // output reg [15:0] wb_reg1_src_indicator,
        // output reg [15:0] wb_reg2_src_indicator

        // output reg [31:2] memif_addr,
        // output reg memif_we,
        // output reg io_we,
        // output reg [3:0] mem_be
    );
    
    rv32_ex_top _rv32_ex_top (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // from id
        .pc_in(_rv32_id_top.pc_out),
        .iw_in(_rv32_id_top.iw_out),
        .rs1_data_in(_rv32_id_top.regif_rs1_data_out),
        .rs2_data_in(_rv32_id_top.regif_rs2_data_out),
        .wb_reg_in(_rv32_id_top.wb_reg_out),
        .wb_enable_in(_rv32_id_top.wb_enable_out),
        .memif_we_in(_rv32_id_top.memif_we_out),
        .io_we_in(_rv32_id_top.io_we_out),
        .mem_be_in(_rv32_id_top.mem_be_out)

        //to id
        // output [31:0] alu_raw,

        // to mem
        // output reg [31:0] pc_out,
        // output reg [31:0] iw_out,
        // output reg [31:0] alu_out,
        // output reg [3:0] wb_reg_out,
        // output reg wb_enable_out
    );

    rv32_mem_top _rv32_mem_top (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // from ex
        .pc_in(_rv32_ex_top.pc_out),
        .iw_in(_rv32_ex_top.iw_out),
        .alu_in(_rv32_ex_top.alu_out),
        .wb_reg_in(_rv32_ex_top.wb_reg_out),
        .wb_enable_in(_rv32_ex_top.wb_enable_out),
        .rs2_data_in(_rv32_id_top.regif_rs2_data_out),

        // to wb
        // output reg [31:0] pc_out,
        // output reg [31:0] iw_out,
        // output reg [31:0] alu_out,
        // output reg [4:0] wb_reg_out,
        // output reg wb_enable_out,

        // memory interface for mem and io
        .mem_addr_in(_rv32_ex_top.memif_addr_out),
        .memif_rdata_in(_dual_port_ram.d_rdata),
        .io_rdata_in(_io_memory.rdata),
        .memif_we_in(_rv32_ex_top.memif_we_out),
        .io_we_in(_rv32_ex_top.io_we_out),
        .mem_be_in(_rv32_ex_top.mem_be_out)

        // output reg [31:2] mem_addr_out,
        // output [31:0] memif_rdata_out, // already registered from memory
        // output [31:0] io_rdata_out, // already registered from io
        // output reg memif_we_out,
        // output reg io_we_out,
        // output reg [3:0] mem_be_out,
        // output reg wb_from_alu_out
    );

    rv32_wb_top _rv32_wb_top (
        .clk(`CLK_PIPELINE),
        .reset(reset),

        // from mem
        .pc_in(_rv32_mem_top.pc_out),
        .iw_in(_rv32_mem_top.iw_out),
        .alu_in(_rv32_mem_top.alu_out),
        .wb_reg_in(_rv32_mem_top.wb_reg_out),
        .wb_enable_in(_rv32_mem_top.wb_enable_out),

        // from io/memory
        .memif_rdata(_rv32_mem_top.memif_rdata_out),
        .io_rdata(_rv32_mem_top.io_rdata_out),
        .wb_from_alu_in(_rv32_mem_top.wb_from_alu_out),
        .mem_be(_rv32_mem_top.mem_be_out)

        // register interface
        // output regif_wb_enable,
        // output [4:0] regif_wb_reg,
        // output [31:0] regif_wb_data
    );

endmodule