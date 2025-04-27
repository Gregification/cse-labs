`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/16/2025 05:25:44 PM
// Design Name: 
// Module Name: rv32_id_top
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

`include "defines.vh"

// instruciton decode
module rv32_id_top(
        // system clock and synchronous reset
        input clk,
        input reset,

        // from if
        input [31:0] pc_in,
        input [31:0] iw_in,

        // df from ex
        input [4:0] df_ex_wb_reg,
        input reg [31:0] df_ex_wb_data,
        input df_ex_wb_enable,
        // df from mem
        input [4:0] df_mem_wb_reg,
        input [31:0] df_mem_wb_data,
        input df_mem_wb_enable,
        // df from wb
        input [4:0] df_wb_wb_reg,
        input [31:0] df_wb_wb_data,
        input df_wb_wb_enable,

        // register interface
        output [4:0] regif_rs1_reg,
        output [4:0] regif_rs2_reg,
        input [31:0] regif_rs1_data_in,
        input [31:0] regif_rs2_data_in,

        // to ex
        output reg [31:0] pc_out,
        output reg [31:0] iw_out,
        output reg [4:0] wb_reg_out,
        output reg wb_enable_out,
        output reg [31:0] regif_rs1_data_out,
        output reg [31:0] regif_rs2_data_out,

        // to if : regarding pc jumping
        output reg jump_enable_out,
        output reg [31:0] jump_addr_out,

        // to mem/io through ex
        output reg memif_we_out,
        output reg io_we_out,
        output reg [3:0] mem_be_out,

        // register df from ex
        input df_wb_from_mem_ex,

        // register df from mem
        input df_wb_from_mem_mem,

        output reg stall
    );

    assign regif_rs1_reg = iw_in[19:15];
    assign regif_rs2_reg = iw_in[24:20];

    reg flood_latch;
    initial flood_latch = 1; // processer starts in a locked state until first after reset
    assign wb_reg1_src_indicator = flood_latch;

    reg [31:0] regif_rs1_data;
    reg [31:0] regif_rs2_data;

    wire    [6:0]  opcode  = iw_in[6:0];
    wire    [2:0]  funct3  = iw_in[14:12];
    wire    [4:0]  rs1     = iw_in[19:15];
    wire    [4:0]  rs2     = iw_in[24:20];
    wire    [31:0] i_B     = {{21{iw_in[31]}}, iw_in[31], iw_in[7], iw_in[30:25], iw_in[11:8]};
    wire    [31:0] i_I     = {{20{iw_in[31]}}, iw_in[31:20]};
    wire    [31:0] i_J     = {{12{iw_in[31]}}, iw_in[31], iw_in[19:12], iw_in[20], iw_in[30:21]};

    // mem & io
    always_ff @ (posedge clk) begin
        io_we_out <= 0;
        memif_we_out <= 0;
        mem_be_out <= 0;

        if(reset) begin
            mem_be_out <= 4'b1111;
        end else begin

            // if is store operation
            if (opcode == 7'b0100011) begin
                io_we_out <= 1;
                memif_we_out <= 1;
            end

            case (funct3)
                3'b100,3'b000:mem_be_out <= 4'b0001; // LB : load byte U/S
                3'b101,3'b001:mem_be_out <= 4'b0011; // LH : load halfword U/S
                3'b010:mem_be_out <= 4'b1111;// LW : load word signed
                default: mem_be_out <= 4'b0000;
            endcase
        end
    end

    assign stall = reset ? 0 : (df_wb_from_mem_ex || df_wb_from_mem_mem);

    always_ff @ (posedge clk) begin
        jump_enable_out <= 0;
        jump_addr_out <= 0;
        regif_rs1_data_out <= 0;
        regif_rs2_data_out <= 0;

        if(reset) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;

            regif_rs1_data_out <= 0;
            regif_rs2_data_out <= 0;

            flood_latch <= 0;

        end else if(flood_latch || stall) begin
            iw_out <= `IW_RESET;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;

        end else begin

            pc_out <= pc_in;

            iw_out <= iw_in;

            wb_reg_out      <= iw_in[11:7];
            wb_enable_out   <= 
                (    opcode == 7'b0110011    // R type : orange
                    ||  opcode == 7'b0000011    // I type subsection : dull red
                    ||  opcode == 7'b0010011    // ^ : yellow
                    ||  opcode == 7'b0110111    // U type : dull blue
                    ||  opcode == 7'b0010111    // U type : dark green
                )
                    && (iw_in[11:7] != 0)
                ;

            regif_rs1_data_out <= regif_rs1_data;
            regif_rs2_data_out <= regif_rs2_data;

            case (opcode)
                7'b1110011: begin // I type : gray
                    if(iw_in[20]) begin
                        // EBREAK
                        if(!jump_enable_out)
                            flood_latch <= 1;
                    end else begin
                        // ECALL     //TODO
                    end
                end // I type : gray

                7'b1100111: begin // I type : dull blue
                    // JALR : jump to address relative to register value
                    jump_enable_out <= 1;

                    // in ex
                    // rd <- pc + 4

                    // pc ← rs1+signex(i[11:0]) (zero lsb)
                    jump_addr_out[31:1] <= $signed(i_I) + regif_rs1_data;
                    jump_addr_out[0] <= 0;
                end // I type : dull blue
        
                7'b1101111: begin // J type : ilme green
                    // JAL : jump to address relative to PC
                    jump_enable_out <= 1;

                    // in ex
                    // rd <- pc + 4

                    // pc ← pc+2*signex(i[20:1])
                    jump_addr_out <= pc_in + 2 * $signed(i_J); 
                end // J type : ilme green
                

                7'b1100011: begin // B type : white
                    case (funct3)
                        3'b000: // BEQ
                            if($signed(regif_rs1_data) == $signed(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end

                        3'b001: // BNE
                            if($signed(regif_rs1_data) != $signed(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end

                        3'b100: // BLT
                            if($signed(regif_rs1_data) < $signed(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end

                        3'b101: // BGE
                            if($signed(regif_rs1_data) >= $signed(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end

                        3'b110: // BLTU
                            if($unsigned(regif_rs1_data) < $unsigned(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end

                        3'b111: // BGEU
                            if($unsigned(regif_rs1_data) >= $unsigned(regif_rs2_data)) begin
                                jump_enable_out <= 1;
                                // pc←pc + 2*signex(i[12:1])
                                jump_addr_out <= pc_in + 2 * $signed(i_B);
                            end
                    endcase // funct3
                end // B type : white
            endcase // opcode
        end
    end

    always_comb begin
        regif_rs1_data = 
            (df_ex_wb_enable    && (df_ex_wb_reg == regif_rs1_reg))     ? df_ex_wb_data :
            (df_mem_wb_enable   && (df_mem_wb_reg == regif_rs1_reg))    ? df_mem_wb_data :
            (df_wb_wb_enable    && (df_wb_wb_reg == regif_rs1_reg))     ? df_wb_wb_data : 
            regif_rs1_data_in;

        regif_rs2_data = 
            (df_ex_wb_enable    && (df_ex_wb_reg == regif_rs2_reg))     ? df_ex_wb_data :
            (df_mem_wb_enable   && (df_mem_wb_reg == regif_rs2_reg))    ? df_mem_wb_data :
            (df_wb_wb_enable    && (df_wb_wb_reg == regif_rs2_reg))     ? df_wb_wb_data : 
            regif_rs2_data_in;
    end

endmodule