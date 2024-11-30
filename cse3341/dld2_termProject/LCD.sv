
module LCD
	#(parameter                  WIDTH       = 32                          , // Bit Width
	  parameter                  DIGITS      = 16                          , // Digits
	  parameter                  FLOAT       = 0                           , // Floating Point Enable
	  parameter                  MODE        = 1                           , // 0 = ADD,SUB,AND,XOR | 1 = ADD,SUB,MUL,DIV
	  parameter                  LINES       = 4                           , // Number of Lines on the LCD
	  parameter                  CHARS       = 20                          , // Number of Characters per line on the LCD
	  parameter [0:LINES-1][6:0] LINE_STARTS = {7'h00, 7'h40, 7'h14, 7'h54}) // Line Starting Addresses
	 (input                      clk                                       , // 50MHz Clock
	  inout    [7:0]             lcd_data                                  , // LCD Data Bus
	  output                     lcd_rs                                    , // LCD Register Select
	  output                     lcd_rw                                    , // LCD Read Write Select
	  output                     lcd_e                                     , // LCD Execute
	  input                      lcd_reset                                 , // LCD Reset
	  input    [WIDTH-1:0]       A                                         , // Value A
	  input    [WIDTH-1:0]       B                                         , // Value B
	  input    [WIDTH-1:0]       C                                         , // Value C
	  input    [1:0]             Operation                                ); // Operation

	wire [7:0] display_chars [0:LINES-1][0:CHARS-1];
	
	wire a_sign, b_sign, c_sign;
	wire [3:0] a_bcd   [DIGITS-1:0];
	wire [3:0] b_bcd   [DIGITS-1:0];
	wire [3:0] c_bcd   [DIGITS-1:0];
	wire [7:0] a_disp  [CHARS-1:0];
	wire [7:0] b_disp  [CHARS-1:0];
	wire [7:0] c_disp  [CHARS-1:0];
	wire [7:0] op_disp [CHARS-1:0];
	
	f322bcd #(
		.WIDTH(WIDTH),
		.DIGITS(DIGITS)
		) a_bin2bcd (
		.bin(A),
		.sign(a_sign),
		.bcd(a_bcd)
	);
	
	bcd2disp #(
		.WIDTH(WIDTH),
		.CHARS(CHARS),
		.DIGITS(DIGITS)
		) a_bcd2disp (
		.sign(a_sign),
		.bcd(a_bcd),
		.disp(a_disp)
	);
	
	f322bcd #(
		.WIDTH(WIDTH),
		.DIGITS(DIGITS)
		) b_bin2bcd (
		.bin(B),
		.sign(b_sign),
		.bcd(b_bcd)
	);
	
	bcd2disp #(
		.WIDTH(WIDTH),
		.CHARS(CHARS),
		.DIGITS(DIGITS)
		) b_bcd2disp (
		.sign(b_sign),
		.bcd(b_bcd),
		.disp(b_disp)
	);
	
	f322bcd #(
		.WIDTH(WIDTH),
		.DIGITS(DIGITS)
		) c_bin2bcd (
		.bin(C),
		.sign(c_sign),
		.bcd(c_bcd)
	);
	
	bcd2disp #(
		.WIDTH(WIDTH),
		.CHARS(CHARS),
		.DIGITS(DIGITS)
		) c_bcd2disp (
		.sign(c_sign),
		.bcd(c_bcd),
		.disp(c_disp)
	);
	
	calculator_operation_display #(
		 .MODE(MODE),
		 .CHARS(CHARS)
		 )(
		 .Operation(Operation),
		 .disp(op_disp)
	);

	wire initilizer_done, initilizer_rs, initilizer_rw, initilizer_e;
	wire [7:0] initilizer_data;

	LCD_Initilizer (
		.clk(clk),
		.reset(lcd_reset),
		.RS(initilizer_rs),
		.RW(initilizer_rw),
		.E(initilizer_e),
		.DATA(initilizer_data),
		.done(initilizer_done)
	);
	
	wire driver_rs, driver_rw, driver_e;
	wire [7:0] driver_data;
	
	assign display_chars[0] = a_disp;
	assign display_chars[1] = b_disp;
	assign display_chars[2] = op_disp;
	assign display_chars[3] = c_disp;
	
	LCD_Driver #(
		 .LINES(LINES),
		 .CHARS(CHARS),
		 .LINE_STARTS(LINE_STARTS)
		 )(
		.clk(clk),
		.reset(lcd_reset),
		.initilized(initilizer_done),
		.RS(driver_rs),
		.RW(driver_rw),
		.E(driver_e),
		.DATA(driver_data),
		.display_chars(display_chars)
	);
	
	assign lcd_rs = !initilizer_done ? initilizer_rs : driver_rs;
	assign lcd_rw = !initilizer_done ? initilizer_rw : driver_rw;
	assign lcd_e = !initilizer_done ? initilizer_e : driver_e;
	assign lcd_data = !initilizer_done ? initilizer_data : driver_data;

endmodule

module calculator_operation_display
	#(parameter            MODE        = 1       , // 0 = ADD,SUB,AND,XOR | 1 = ADD,SUB,MUL,DIV
	  parameter            CHARS       = 20      ) // Number of Characters per line on the LCD
	 (input        [1:0]   Operation             , // Operation
	  output   reg [7:0]   disp      [CHARS-1:0]); // Output Display Data
	  

	integer i;

	always @(Operation) begin
		for (i = CHARS-1; i >= 0; i = i - 1) begin
			disp[i] = CHAR_LOW_LINE;
		end
		
		if(MODE == 1) begin
			case (Operation)
				3'b001: begin
//						disp[0+:4] 	= {CHAR_CAPITAL_P,CHAR_SMALL_L,CHAR_SMALL_U,CHAR_SMALL_S};
						//cant get that to work so brute forcing it
						disp[CHARS-1] = CHAR_CAPITAL_P;
						disp[CHARS-2] = CHAR_SMALL_L;
						disp[CHARS-3] = CHAR_SMALL_U;
						disp[CHARS-4] = CHAR_SMALL_S;
					end
				3'b010:   begin 
//						disp[0+:5] 	= {CHAR_CAPITAL_M,CHAR_SMALL_I,CHAR_SMALL_N,CHAR_SMALL_U,CHAR_SMALL_S};
						disp[CHARS-1] = CHAR_CAPITAL_M;
						disp[CHARS-2] = CHAR_SMALL_I;
						disp[CHARS-3] = CHAR_SMALL_N;
						disp[CHARS-4] = CHAR_SMALL_U;
						disp[CHARS-5] = CHAR_SMALL_S;
					end
				default:;
			endcase
		end
	end
	
endmodule