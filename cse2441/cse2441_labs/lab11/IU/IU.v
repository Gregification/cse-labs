//lab11 input unit
module IU(
			input CLOCK,
			input RESET,
			input [3:0] row,
			output [3:0] col,
			output reg [7:0] twosComp,
			output reg isValid,
			
			output [3:0] o_LAST_BCD,
			output o_trig
		);
		
		wire [15:0] BCD;
		wire [7:0] binarySM;

		always begin
			if(binarySM[7] == 1)
				twosComp = ~binarySM + 1'b1;
			else
				twosComp = binarySM;
				
			twosComp[7] = binarySM[7];
			
			if(BCD[11:8]*(8'b01100100) + BCD[7:4]*(8'b1010) + BCD[3:0] > 127)
				isValid = 1'b0;
			else 
				isValid = 1'b1;
		end
		
		
		keypad_input k_in(
			.row(row),
			.clk(CLOCK),
			.reset(RESET),
			
			.col(col),
			.out(BCD),
			.value(o_LAST_BCD),
			.trig(o_trig)
		);
		
		BCD2BinarySM bcd_2_sm(
			.BCD(BCD),
			.binarySM(binarySM)
		);

endmodule