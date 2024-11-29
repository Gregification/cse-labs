module bcd2disp
	#(parameter            WIDTH  = 32                                   , // Bit Width
	  parameter            CHARS  = 20                                   , // Disp Char Width
	  parameter            DIGITS = 10                                   ) // Digits
	 (input                sign                                          , // Sign Bit
	  input        [3:0]   bcd        [DIGITS-1:0]                       , // BCD Output
	  output   reg [7:0]   disp       [CHARS-1:0]                       ); // Output Display Data
	
	integer i;
	
	logic leading_zero;

	always @(sign, bcd) begin
		leading_zero = 1;
		if (DIGITS > (CHARS-1)) begin
			for (i = CHARS-1; i >= 0; i = i - 1) begin
				disp[i] = CHAR_ASTERISK;
			end
		end else begin
			for (i = CHARS-1; i >= 0; i = i - 1) begin
				if (i < DIGITS) begin
					if (leading_zero == 1 && bcd[i] == 4'd0) begin
						disp[i] = CHAR_BLANK;
					end else begin
						leading_zero = 0;
						case(bcd[i])
							4'd0:    disp[i] = CHAR_0;
							4'd1:    disp[i] = CHAR_1;
							4'd2:    disp[i] = CHAR_2;
							4'd3:    disp[i] = CHAR_3;
							4'd4:    disp[i] = CHAR_4;
							4'd5:    disp[i] = CHAR_5;
							4'd6:    disp[i] = CHAR_6;
							4'd7:    disp[i] = CHAR_7;
							4'd8:    disp[i] = CHAR_8;
							4'd9:    disp[i] = CHAR_9;
							4'hF:		disp[i] = CHAR_PERIOD;
							default: disp[i] = CHAR_ASTERISK;
						endcase
					end
				end else if (i == CHARS-1) begin
					disp[CHARS-1] = (sign == 1) ? CHAR_HYPHEN_MINUS : CHAR_BLANK;
				end else begin
					disp[i] = CHAR_BLANK;
				end
			end
		end
	end

endmodule