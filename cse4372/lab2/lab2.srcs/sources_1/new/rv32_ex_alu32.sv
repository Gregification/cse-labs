`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/02/2025 02:28:31 PM
// Design Name: 
// Module Name: rv32_ex_alu32
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

`define DEFAULT_COMBO_STATE alu_out <= 0;

module rv32_ex_alu32(        
        // from id
        input [31:0] pc_in,
        input [31:0] iw_in,         // instruction word
        input signed [31:0] rs1_data_in,
        input signed [31:0] rs2_data_in,
        
        // to mem
        output reg [31:0] alu_out
    );

    // all iw components  
    wire        [6:0]  opcode  = iw_in[6:0];
    wire        [4:0]  rd      = iw_in[11:7];
    wire        [2:0]  funct3  = iw_in[14:12];
    wire signed [4:0]  rs1     = iw_in[19:15];
    wire signed [4:0]  rs2     = iw_in[24:20];
    wire        [4:0]  shamt   = rs2;
    wire        [6:0]  funct7  = iw_in[31:25];
    wire signed [31:0] i_I     = {{20{iw_in[31]}}, iw_in[31:20]};
    wire signed [31:0] i_S     = {{20{iw_in[31]}}, iw_in[31:25], iw_in[11:7]};
    wire signed [31:0] i_B     = {{19{iw_in[31]}}, iw_in[31], iw_in[7], iw_in[30:25], iw_in[11:8], 1'b0};
    wire        [31:0] i_U     = {iw_in[31:12], 12'b0};
    wire signed [31:0] i_J     = {{11{iw_in[31]}}, iw_in[31], iw_in[19:12], iw_in[20], iw_in[30:21], 1'b0};

    // latch prevention
    initial begin
        `DEFAULT_COMBO_STATE
    end

    // iw parsing
    //      color coding & naming based off the courses excel sheet
    always_comb begin
        // latch prevention
        `DEFAULT_COMBO_STATE

        case (opcode)
            7'b0110011: begin   // R type : orange
                case (funct3)
                    3'b000: alu_out <= funct7[5] ? rs1_data_in - rs2_data_in : rs1_data_in + rs2_data_in; // ADD, SUB
                    3'b001: alu_out <= rs1_data_in << rs2[4:0]; // SLL : shift left logical
                    3'b010: alu_out <= rs1_data_in < rs2_data_in; // SLT : store 1 if signed less than
                    3'b011: alu_out <= $unsigned(rs1_data_in) < $unsigned(rs2_data_in); // SLTU : store 1 if less than unsigned
                    3'b100: alu_out <= rs1_data_in ^ rs2_data_in; // XOR
                    3'b101: if(iw_in[30] == 1) 
                            alu_out <= rs1_data_in >>> rs2[4:0]; // SRA : shift right arithmetic
                        else
                            alu_out <= rs1_data_in >> rs2[4:0]; // SRL : shift right logical
                    3'b110: alu_out <= rs1_data_in | rs2_data_in; // OR
                    3'b111: alu_out <= rs1_data_in & rs2_data_in; // AND
                endcase // funct3
            end // R type : orange
            
            7'b1100111: begin // I type : dull blue
                // JALR : jump to address relative to register value
                // rd <- pc + 4 //TODO

                // pc
                alu_out <= {rs1_data_in[31:1], 1'b0} + i_I;
            end // I type : dull blue

            7'b0000011: begin // I type : dull red
                case (funct3)
                    3'b000, // LB : load byte signed
                    3'b001, // LH : load halfword signed
                    3'b010, // LW : load word signed
                    3'b100, // LBU : load byte unsigned
                    3'b101: // LHU : load halfword unsigned
                        alu_out <= rs1_data_in + i_I;
                endcase // funct3
            end // I type : dull red

            7'b0010011: begin // I type : yellow
                case (funct3)
                    3'b000: alu_out <= rs1_data_in + i_I; // ADDI
                    3'b001: alu_out <= rs1_data_in << shamt; // SLLI : shift left logical immediate
                    3'b010: alu_out <= rs1_data_in < i_I; // SLTI : store 1 if signed less than immediate
                    3'b011: alu_out <= $unsigned(rs1_data_in) < $unsigned(i_I); // SLTIU : store 1 if less than unsigned immediate
                    3'b100: alu_out <= rs1_data_in ^ i_I; // XORI
                    3'b101: if(iw_in[30] == 1) 
                            alu_out <= rs1_data_in >> shamt; // SRLI : shift right logical immediate
                        else
                            alu_out <= rs1_data_in >>> shamt; // SRAI : shift right arithmetic immediate
                    3'b110: alu_out <= rs1_data_in | i_I; // ORI
                    3'b111: alu_out <= rs1_data_in & i_I; // ANDI
                endcase // funct 3
            end // I type : yellow

            // 7'b0001111: begin // I type : white
            //     // FENCE     //TODO
            // end // I type : white

            // 7'b1110011: begin // I type : gray
            //     if(iw_in[20]) begin
            //         // ECALL     //TODO
            //     end else begin
            //         // EBREAK    //TODO
            //     end
            // end // I type : gray

            7'b0100011: begin // S type : dull pruple
                case (funct3)
                    3'b000, // SB : store byte
                    3'b001, // SH : store halfword
                    3'b010: // SW : store word
                        alu_out <= rs1_data_in + i_S;
                endcase // funct3
            end // S type : dull pruple

            7'b1100011: begin // B type : white
                // case (funct3)
                    // 3'b000: // BEQ   //TODO
                    // 3'b001: // BNE   //TODO
                    // 3'b100: // BLT   //TODO
                    // 3'b101: // BGE   //TODO
                    // 3'b110: // BLTU  //TODO
                    // 3'b111: // BGEU  //TODO
                // endcase // funct3
            end // B type : white

            7'b0110111: begin // U type : dull blue
                alu_out <= {i_U[31:12], 12'b0}; // LUI : load upper immediate
            end // U type : dull blue

            7'b0010111: begin // U type : dark green
                alu_out <= pc_in + {i_U[31:12], 12'b0}; // AUIPC : add upper immediate to PC
            end // U type : dark green

            7'b1101111: begin // J type : ilme green
                alu_out <= pc_in + i_J; // JAL : jump to address relative to PC
            end // J type : ilme green

        endcase // opcode
    end
    
endmodule
