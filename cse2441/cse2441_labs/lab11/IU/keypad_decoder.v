module keypad_decoder #(
        parameter BASE = 10 // Default Base 10 (Options: 10,16)
    ) (
        input [3:0] row,
        input [3:0] col,
        output reg [3:0] value,
        output reg valid // Optional
    );

    always @ (row, col) begin
        if (BASE == 10) begin
            // 1 2 3 A
            // 4 5 6 B
            // 7 8 9 C
            // E 0 F D
            case ({row, col})
                8'b0001_0001: begin value = 1; valid = 1; end // 1
                8'b0001_0010: begin value = 2; valid = 1; end // 2
                8'b0001_0100: begin value = 3; valid = 1; end // 3
                8'b0001_1000: begin value = 10; valid = 1; end // A
                8'b0010_0001: begin value = 4; valid = 1; end // 4
                8'b0010_0010: begin value = 5; valid = 1; end // 5
                8'b0010_0100: begin value = 6; valid = 1; end // 6
                8'b0010_1000: begin value = 11; valid = 1; end // B
                8'b0100_0001: begin value = 7; valid = 1; end // 7
                8'b0100_0010: begin value = 8; valid = 1; end // 8
                8'b0100_0100: begin value = 9; valid = 1; end // 9
                8'b0100_1000: begin value = 12; valid = 1; end // C
                8'b1000_0001: begin value = 14; valid = 1; end // E
                8'b1000_0010: begin value = 0; valid = 1; end // 0
                8'b1000_0100: begin value = 15; valid = 1; end // F
                8'b1000_1000: begin value = 13; valid = 1; end // D
                default: begin value = 0; valid = 0; end // Misc
            endcase
        end
        else if (BASE == 16) begin
            // 0 1 2 3
            // 4 5 6 7
            // 8 9 A B
            // C D E F
            case ({row, col})
                8'b0001_0001: begin value = 0; valid = 1; end // 0
                8'b0001_0010: begin value = 1; valid = 1; end // 1
                8'b0001_0100: begin value = 2; valid = 1; end // 2
                8'b0001_1000: begin value = 3; valid = 1; end // 3
                8'b0010_0001: begin value = 4; valid = 1; end // 4
                8'b0010_0010: begin value = 5; valid = 1; end // 5
                8'b0010_0100: begin value = 6; valid = 1; end // 6
                8'b0010_1000: begin value = 7; valid = 1; end // 7
                8'b0100_0001: begin value = 8; valid = 1; end // 8
                8'b0100_0010: begin value = 9; valid = 1; end // 9
                8'b0100_0100: begin value = 10; valid = 1; end // A
                8'b0100_1000: begin value = 11; valid = 1; end // B
                8'b1000_0001: begin value = 12; valid = 1; end // C
                8'b1000_0010: begin value = 13; valid = 1; end // D
                8'b1000_0100: begin value = 14; valid = 1; end // E
                8'b1000_1000: begin value = 15; valid = 1; end // F
                default: begin value = 0; valid = 0; end // Misc
            endcase
        end
        else begin
            value = 0; valid = 0; // Invalid Base
        end
    end
endmodule