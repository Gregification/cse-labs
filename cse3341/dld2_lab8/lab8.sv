/**
	cse3341, digital logic 2, lab 8
	George Boone
	1002055713
	
	Code baselined from Quartus "LCD" and "LCD test" projects posted in canvas as suggested
*/

module lab8
	 (input                clk          , // 50MHz Clock
	  input                lcd_reset    , // LCD Reset
	  input    [0:11] 	  LCD_Board_PB , // LCD Board Push Buttons
     input    [0:4] 		  LCD_Board_SW , // LCD Board Switches
	  inout    [7:0]       lcd_data     , // LCD Data Bus
     output   [0:7] 		  LCD_Board_LED, // LCD Board LEDs
     output   [0:9] 		  DE10_LED		, // DE10_Lite Red LEDs
	  output               lcd_rs       , // LCD Register Select
	  output               lcd_rw       , // LCD Read Write Select
	  output               lcd_e          // LCD Execute
	  );  
	  
	 assign DE10_LED[0:9]      = LCD_Board_PB[0:9];
    assign LCD_Board_LED[0]   = LCD_Board_SW[0];
    assign LCD_Board_LED[1]   = LCD_Board_SW[1];
    assign LCD_Board_LED[2]   = LCD_Board_SW[2];
    assign LCD_Board_LED[3]   = LCD_Board_SW[3];
    assign LCD_Board_LED[4]   = LCD_Board_SW[4];
	 assign LCD_Board_LED[5]   = LCD_Board_PB[10];
	 assign LCD_Board_LED[6]   = LCD_Board_PB[11];  
	 assign LCD_Board_LED[7]   = LCD_Board_PB[11];  
	  
	LCD #(
		 .WIDTH(64),
		 .DIGITS(19),
		 .FLOAT(0),
		 .MODE(1),
		 .LINES(4),
		 .CHARS(20),
		 .LINE_STARTS({7'h00, 7'h40, 7'h14, 7'h54})
		 )(
		 .clk(clk),
		 .lcd_data(lcd_data),
		 .lcd_rs(lcd_rs),
		 .lcd_rw(lcd_rw),
		 .lcd_e(lcd_e),
		 .lcd_reset(!lcd_reset),
		 .A(32'hABCD),
		 .B(32'h1),
		 .C(32'hAAAA),
		 .Operation(2'b10) // PLUS
	);

endmodule
