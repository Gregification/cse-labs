/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
*/

module dld2_termProject (
			// lcd
			input             CLK          , // 50MHz Clock
			input    [0:11]	LCD_Board_PB , // LCD Board Push Buttons
			input    [0:4] 	LCD_Board_SW , // LCD Board Switches
			inout    [7:0]    lcd_data     , // LCD Data Bus
			output   [0:7] 	LCD_Board_LED, // LCD Board LEDs
			output   [0:9] 	DE10_LED		, // DE10_Lite Red LEDs
			output            lcd_rs       , // LCD Register Select
			output            lcd_rw       , // LCD Read Write Select
			output            lcd_e,          // LCD Execute
			
			// keypad
			input 	[3:0] 	KEYPAD_ROWS,
			output	[3:0] 	KEYPAD_COLS,
			
			// devboard
			input [9:0] DEVBOARD_SWS,
			input [1:0] DEVBOARD_BTN
		
	  );
	
	wire lcd_reset;
	 
	//----------------------------io---------------------------------
	assign lcd_reset = DEVBOARD_BTN[0];
	
	assign DE10_LED[0:9]      = LCD_Board_PB[0:9];
	assign LCD_Board_LED[0]   = LCD_Board_SW[0];
	assign LCD_Board_LED[1]   = LCD_Board_SW[1];
	assign LCD_Board_LED[2]   = LCD_Board_SW[2];
	assign LCD_Board_LED[3]   = LCD_Board_SW[3];
	assign LCD_Board_LED[4]   = LCD_Board_SW[4];
	assign LCD_Board_LED[5]   = LCD_Board_PB[10];
	assign LCD_Board_LED[6]   = LCD_Board_PB[11];  
	assign LCD_Board_LED[7]   = LCD_Board_PB[11];  

	//--------------------------internal-----------------------------
	reg 	[1:0][31:0]	float_vals;
	
	//----------------------------ui---------------------------------
	reg 			sel_row;
	
	always_ff @ (negedge LCD_Board_PB[0+:4]) begin
		if(LCD_Board_PB[0] == 0)
			sel_row = sel_row + 1;
		else if(LCD_Board_PB[3] == 0)
			sel_row = sel_row - 1;
		
		if(LCD_Board_PB[1] == 0)
			float_vals[sel_row][23+:8] = float_vals[sel_row][23+:8] + 1;
		else if(LCD_Board_PB[2] == 0)
			float_vals[sel_row][23+:8] = float_vals[sel_row][23+:8] - 1;
	end
	
	LCD #(
		 .WIDTH(64),
		 .DIGITS(19),
		 .FLOAT(0),
		 .MODE(1),
		 .LINES(4),
		 .CHARS(20),
		 .LINE_STARTS({7'h00, 7'h40, 7'h14, 7'h54})
		 )(
		 .clk(CLK),
		 .lcd_data(lcd_data),
		 .lcd_rs(lcd_rs),
		 .lcd_rw(lcd_rw),
		 .lcd_e(lcd_e),
		 .lcd_reset(!lcd_reset),
		 .A(32'h3fc00000),
		 .B(32'd4),
		 .C(32'hc0080000),
		 .Operation(2'b01) // PLUS
	);

	reg [31:0] clk_ladder;
	always_ff @ (posedge CLK)
		clk_ladder = clk_ladder + 1'b1;
	
	KeyPad #(
			.N(4)
		) __key_pad (
			.CLK(clk_ladder[28]),
			.SHIFT(_pressed),
			.RESET(RESET),
			
			.ROWS(KEYPAD_ROWS),
			
			.COLS(KEYPAD_COLS),
			.PRESSED(_pressed),
			.NXTVAL(_val),
			.VALUE(_value)
		);

endmodule

