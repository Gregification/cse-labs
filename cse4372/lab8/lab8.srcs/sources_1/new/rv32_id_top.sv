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

        // debugging to top
        output reg [15:0] wb_reg1_src_indicator,
        output reg [15:0] wb_reg2_src_indicator
    );

    assign regif_rs1_reg = iw_in[19:15];
    assign regif_rs2_reg = iw_in[24:20];

    reg flood_latch;
    initial flood_latch = 1; // processer starts in a locked state until first after reset

    reg [31:0] regif_rs1_data;
    reg [31:0] regif_rs2_data;

    wire    [6:0]  opcode  = iw_in[6:0];
    wire    [2:0]  funct3  = iw_in[14:12];
    wire    [4:0]  rs1     = iw_in[19:15];
    wire    [4:0]  rs2     = iw_in[24:20];
    wire    [31:0] i_B     = {{21{iw_in[31]}}, iw_in[31], iw_in[7], iw_in[30:25], iw_in[11:8]};
    wire    [31:0] i_I     = {{20{iw_in[31]}}, iw_in[31:20]};
    wire    [31:0] i_J     = {{12{iw_in[31]}}, iw_in[31], iw_in[19:12], iw_in[20], iw_in[30:21]};

    always_ff @ (posedge clk) begin
        jump_enable_out <= 0;
        jump_addr_out <= 0;

        if(reset || flood_latch) begin
            pc_out <= `PC_RESET;
            iw_out <= `IW_RESET;

            wb_reg_out      <= 0;
            wb_enable_out   <= 0;

            regif_rs1_data_out <= 0;
            regif_rs2_data_out <= 0;

            if(reset) begin
                flood_latch <= 0;
            end

        end else begin
            pc_out <= pc_in;

            if(jump_enable_out) begin // if is bad instruction
                // replace with nop
                pc_out <= `PC_RESET;
                iw_out <= `NOP_IW;

                wb_reg_out      <= 0;
                wb_enable_out   <= 0;

                regif_rs1_data_out <= 0;
                regif_rs2_data_out <= 0;

            end else begin
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
                            // ECALL     //TODO
                        end else begin
                            // EBREAK
                            flood_latch <= 1;
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
    end

    always_comb begin
        wb_reg1_src_indicator = 
            (df_ex_wb_enable    && (df_ex_wb_reg == regif_rs1_reg))     ? 32'hA0 :  // from ex
            (df_mem_wb_enable   && (df_mem_wb_reg == regif_rs1_reg))    ? 32'hAA :  // from mem
            (df_wb_wb_enable    && (df_wb_wb_reg == regif_rs1_reg))     ? 32'hAB :  // from wb
            32'hAC;                                                                 // no df

        wb_reg1_src_indicator = 
            (df_ex_wb_enable    && (df_ex_wb_reg == regif_rs2_reg))     ? 32'hB0 :
            (df_mem_wb_enable   && (df_mem_wb_reg == regif_rs2_reg))    ? 32'hBA :
            (df_wb_wb_enable    && (df_wb_wb_reg == regif_rs2_reg))     ? 32'hBB : 
            32'hBC;

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