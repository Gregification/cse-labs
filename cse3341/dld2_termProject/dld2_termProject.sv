/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
	
	float23 adder-subtractor
	
	- 2 input modes, toggled by de10 board sw0
		- mantissa & expo : edit 4 specific bits of the matissa controlled by the scroll of the exponent
		- raw input : little endian, MSB first, itterates across entire float in pairs of 4 bits
	- useful for decoding
		https://gregstoll.com/~gregstoll/floattohex/
*/

module dld2_termProject (
			// lcd
			input             CLK          , // 50MHz Clock
			input    [0:11]	LCD_Board_PB , // LCD Board Push Buttons
			input    [0:4] 	LCD_Board_SW , // LCD Board Switches
			inout    [7:0]    lcd_data     , // LCD Data Bus
			output   [0:7] 	LCD_Board_LED, // LCD Board LEDs
			output   [0:9] 	DE10_LED		 , // DE10_Lite Red LEDs
			output            lcd_rs       , // LCD Register Select
			output            lcd_rw       , // LCD Read Write Select
			output            lcd_e,         // LCD Execute
			
			// keypad
			input 	[3:0] 	KEYPAD_ROWS,
			output	[3:0] 	KEYPAD_COLS,
			
			// devboard
			input [9:0] DEVBOARD_SWS,
			input [1:0] DEVBOARD_BTN
		
	  );
	//------------------------internal-------------------------------
	 
	reg 	[31:0] 		clk_ladder;
	reg 	[2:0][31:0]	float_vals;
	reg	[1:0]			opearation;
	reg	[3:0]			input_val;
	reg 					sel_row;
	wire 					lcd_reset;
	wire	[7:0]			sel_expo;
	wire	[23:0]		sel_mantissa;
	//jank code
	wire					raw_input_mode;
		reg[2:0]			raw_input_counter;
	 
	//----------------------------io---------------------------------
	
	wire 					_pressed;
	wire					overflow, underflow;
	wire					add_sub;					//0:add
	
	assign lcd_reset = DEVBOARD_BTN[0];
	
	// array assignments get reversed for some reason ... at this point i just want this to work
	reg [3:0] valCViewReversed;
	assign valCViewReversed[0] = LCD_Board_SW[0];
	assign valCViewReversed[1] = LCD_Board_SW[1];
	assign valCViewReversed[2] = LCD_Board_SW[2];
	assign LCD_Board_LED[0] = valCViewReversed[0];
	assign LCD_Board_LED[1] = valCViewReversed[1];
	assign LCD_Board_LED[2] = valCViewReversed[2];
	assign LCD_Board_LED[4] = float_vals[2] >> (valCViewReversed*4);
	assign LCD_Board_LED[5] = float_vals[2] >> (valCViewReversed*4+1);
	assign LCD_Board_LED[6] = float_vals[2] >> (valCViewReversed*4+2);
	assign LCD_Board_LED[7] = float_vals[2] >> (valCViewReversed*4+3);
	
	assign DE10_LED[9]   = overflow;  
	assign DE10_LED[8]   = underflow;  
	
	assign sel_expo		= float_vals[sel_row][23+:8];
	assign sel_mantissa	= float_vals[sel_row][0+:23];
	
	always_ff @ (posedge CLK)
		clk_ladder += 1;
	
	
	//----------------------------ui---------------------------------	
	
	wire	[7:0]						debug;
	
	assign raw_input_mode		= DEVBOARD_SWS[0];
	assign add_sub					= DEVBOARD_SWS[1];
	assign debug					= DEVBOARD_SWS[2+:8];
	
	assign DE10_LED[0]			= raw_input_mode;
	assign DE10_LED[1]			= add_sub;
	assign DE10_LED[2]			= |raw_input_counter;
	
	always_ff @ (negedge _pressed) begin
		raw_input_counter += 1;
		
		if(LCD_Board_PB[0] | LCD_Board_PB[3])
			raw_input_counter = 0;
	end
	
	always_ff @ (posedge clk_ladder[22], posedge _pressed) begin
		static reg ltch;
		
		if(_pressed) begin
			if(ltch) begin
				ltch = 0;
			
				if(raw_input_mode) begin
					float_vals[sel_row] 	&= ~(4'hF		<< (28 - raw_input_counter*4));
					float_vals[sel_row] 	|=  input_val 	<< (28 - raw_input_counter*4);
				end else if(sel_expo >= 127) begin
					float_vals[sel_row][0+:23] &= ~(4'hF		<< (23-4-(sel_expo-127)));
					float_vals[sel_row][0+:23] |=  (input_val << (23-4-(sel_expo-127))) & ~23'b0;
				end
			
			end
		end else begin
			ltch = 1;
			
			if(|LCD_Board_PB[0+:4]) begin
				
				if(LCD_Board_PB[2]) begin
					float_vals[sel_row][23+:8] += 1;
				end
				
				if(LCD_Board_PB[1]) begin
					float_vals[sel_row][23+:8] -= 1;
				end
				
			end else if(LCD_Board_PB[4])
				float_vals[sel_row][23+:8] = 8'd127;
			else if(LCD_Board_PB[5])
				float_vals[sel_row]			= 0;
				
		end
	end
	
	// DO NOT CHANGE, it works. it however will not work if moved to the statement above
	always_ff @ (negedge |LCD_Board_PB[0+:4]) begin
		if(LCD_Board_PB[0] != 0)
			sel_row -= 1;
		if(LCD_Board_PB[3] != 0)
			sel_row += 1;
	end
	
	always_ff @ (negedge DEVBOARD_BTN[1])
		case(opearation)
			2'b01 : opearation = 2'b10;
			2'b10	: opearation = 2'b01;
			default:opearation = 2'b10;
		endcase
	
	
	//--------------------------modules------------------------------
	
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
		 .A(float_vals[0]),
		 .B(float_vals[1]),
		 .C(float_vals[2]),
		 .Operation(opearation[1]),
		 
		 .debug(|debug)
	);
	
	KeyPad #(
			.N(1)
		) __key_pad (
			.CLK(clk_ladder[19]),
			.SHIFT(!_pressed),
			.RESET(lcd_reset),
			
			.ROWS(KEYPAD_ROWS),
			
			.COLS(KEYPAD_COLS),
			.PRESSED(_pressed),
			.NXTVAL(input_val)
		);
	
	F32AdderSubtractor(
		.A(float_vals[0]),
		.B(float_vals[1]),
		.OP(add_sub),
		
		.R(float_vals[2]),
		.UNDERFLOW(underflow),
		.OVERFLOW(overflow),
		
		.debug(debug)
	);

endmodule
