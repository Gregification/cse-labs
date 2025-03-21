module LCD_Driver
	#(parameter                  LINES       = 4                           ,
	  parameter                  CHARS       = 20                          ,
	  parameter [0:LINES-1][6:0] LINE_STARTS = {7'h00, 7'h40, 7'h14, 7'h54},
	  parameter						  use_num_map = 0									  )
	 (input                      clk                                       ,
	  input                      reset                                     ,
	  input                      initilized                                ,
	  output                     RS, RW, E                                 ,
	  inout    [7:0]             DATA                                      ,
	  input    [7:0]             display_chars [0:LINES-1][0:CHARS-1]      ,
	  
	  input	  [3:0]				  number_map [0:LINES-1][0:CHARS-1]			  );

	typedef enum logic [3:0] {
		WaitInit,
		WaitBusy1,
		SetPOS,
		WaitBusy2,
		SetChar,
		WaitBusy3,
		Increment
	} StateType;
	
	StateType state, next_state;
	reg [31:0] counter = 0;
	reg [$clog2(LINES):0] line;
	reg [$clog2(CHARS):0] col;

	always @(posedge clk or posedge reset) begin
		if (reset) begin
			state = WaitInit;
			next_state = WaitInit;
			counter = 0;
			line = 0;
			col = 0;
		end else begin
			if (state != next_state) begin
				state = next_state;
				counter = 0; // Reset counter
			end else
				counter = counter + 1; // Increment counter

			case (state)
				WaitInit: if (initilized == 1) next_state = WaitBusy1; // Wait until initilized
				WaitBusy1: `check_busy(SetPOS) // Wait for BF
				SetPOS: `send_command(8'b10000000 | (LINE_STARTS[line] + col), WaitBusy2) // Set POS
				WaitBusy2: `check_busy(SetChar) // Wait for BF
				SetChar: begin
					if(use_num_map) begin
						case(number_map[line][col])
							4'd0	: `set_char(CHAR_BLANK,			WaitBusy3)
							4'd1	: `set_char(CHAR_1,				WaitBusy3)
							4'd2	: `set_char(CHAR_2,				WaitBusy3)
							4'd3	: `set_char(CHAR_3,				WaitBusy3)
							4'd4	: `set_char(CHAR_4,				WaitBusy3)
							4'd5	: `set_char(CHAR_5,				WaitBusy3)
							4'd6	: `set_char(CHAR_6,				WaitBusy3)
							4'd7	: `set_char(CHAR_7,				WaitBusy3)
							4'd8	: `set_char(CHAR_8,				WaitBusy3)
							4'd9	: `set_char(CHAR_9,				WaitBusy3)
							4'd10	: `set_char(CHAR_CAPITAL_A,		WaitBusy3)
							4'd11	: `set_char(CHAR_CAPITAL_B,		WaitBusy3)
							4'd12	: `set_char(CHAR_CAPITAL_C,		WaitBusy3)
							4'd13	: `set_char(CHAR_CAPITAL_D,		WaitBusy3)
							4'd14	: `set_char(CHAR_CAPITAL_E,		WaitBusy3)
							4'd15	: `set_char(CHAR_CAPITAL_F,		WaitBusy3)
							default : `set_char(CHAR_BLANK,			WaitBusy3)
						endcase
					end else
					`set_char(display_chars[line][col], WaitBusy3) // Set Char
				end
				WaitBusy3: `check_busy(Increment) // Wait for BF
				Increment: begin col += 1; if (col >= CHARS) begin col = 0; line += 1; if (line >= LINES) begin line = 0; end end next_state = WaitBusy1; end // Increment address
				default: next_state = WaitInit;
			endcase
		end
	end
endmodule