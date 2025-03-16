/**
cse3341, digital logic 2, term project
George Boone
1002055713

tetris hardcoded at the lcd provided by UTA

- code for core LCD functionality baselined from slides
*/

// significantly larger

module BudgetTetris#(
		parameter 		ROWS = 4	,
		parameter		COLS = 20	,
		parameter [0:ROWS-1][6:0] LINE_STARTS = {7'h00, 7'h40, 7'h14, 7'h54} // Line Starting Addresses
	)(
		// game
		input				g_clk			,
		input	[5:0]		g_ctrl		, // left, right, down, rotate-left, rotate-right, reset-game
		
		// lcd
		input 			lcd_clk		, // LCD clock			
		inout [7:0] 	lcd_data    , // LCD Data Bus
		output         lcd_rs      , // LCD Register Select
		output         lcd_rw      , // LCD Read Write Select
		output         lcd_e       , // LCD Execute
		input          lcd_reset	  // LCD Reset
	);
	reg [3:0] map 	[0:ROWS-1][0:COLS-1];	// map[row][col] = val
	
	//--------------------------tetris logic--------------------------

	parameter block_size = 8;
	logic	[block_size-1:0][1:0]	verts;
	
	wire left, right, down, rot_left, rot_right, reset;
	assign {left, right, down, rot_left, rot_right, reset} = g_ctrl[0+:6];
	
	integer x, y, i, ctrl_x, ctrl_y;
	integer former_x, former_y; // cheese it
	reg [3:0]		counter;
	reg [1:0] 		ctrl_rot;
	
//	function is_clipping(
//			input integer nx = 0,
//			input integer ny = 0,
//			input [1:0] nrot = 2'b0
//		);
//		
//		logic	[block_size-1:0]	vert_bad;		
//		begin				
//			integer ni;
//			for(ni = 0; ni < block_size; ni = ni + 1) begin
//				integer ex = nx + verts[ni][0] * (nrot[0] ? 1 : -1);
//				integer ey = ny + verts[ni][1] * (nrot[1] ? 1 : -1);
//				
//				vert_bad[i] = (ex != nx || ey != ny) &&			// if value changed
//					ex < 0 || ex >= ROWS || ey < 0 || ey >= COLS		// if is out of bounds
//					|| (map[ex][ey] != 0);									// if is overlapping something
//				
//			end
//			
//			is_clipping = |vert_bad;
//		end
//	endfunction
	
	always @ (negedge g_clk, posedge reset) begin //, posedge left, posedge right, posedge down, posedge rot_left, posedge rot_right, posedge reset) begin
		if(reset) begin
			ctrl_x = 0;
			ctrl_y = 0;
		
			for(y = 0; y < COLS; y = y + 1)
				for(x = 0; x < ROWS; x = x + 1)
					map[x][y] <= 0;
					
		end else begin		
			
			if((g_clk == 0) || down) begin
			
				// collapse
				for(y = 0; y < COLS; y = y + 1) begin
					reg [ROWS-1:0] any_bit;
				
					for(x = 0; x < ROWS; x = x + 1) begin
						any_bit[x] = |map[x][y];
						
						if(y > 0)
							if(map[x][y] == 0 && map[x][y-1] != 0) begin
								map[x][y] 	<= map[x][y-1];
								map[x][y-1]	<= 0;
								
								if(x == ctrl_x && y-1 == ctrl_y) begin
									ctrl_y = y;
									map[ctrl_x][ctrl_y] <= 15;
								end
									
							end else if(x == ctrl_x && y == ctrl_y) begin
								ctrl_y = 0;
							end  
					end
					
					// pop row 
					// if each column of row had something
					if(&any_bit)	
						for(x = 0; x < ROWS; x += 1) begin
							map[x][y] <= map[x][y] - 1;
						end
					
				end
			
				
//				if(left)// && !is_clipping(ctrl_x+1, ctrl_y, ctrl_rot))

				if(left && ctrl_x+1 < ROWS)// && map[ctrl_x+1][ctrl_y] == 0)
					ctrl_x += 1;
				
				if(right && ctrl_x > 0)// && map[ctrl_x-1][ctrl_y] == 0)
					ctrl_x -= 1;
					
			end
			
			if(map[ctrl_x][ctrl_y] == 0) begin
				map[ctrl_x][ctrl_y] <= counter;
				counter = 1;
			end
			
		end
	end
	
	
	//--------------------------LCD driving---------------------------
	
	wire initilizer_done, initilizer_rs, initilizer_rw, initilizer_e;
	wire [7:0] initilizer_data;
	
	LCD_Initilizer (
		.clk	(lcd_clk),
		.reset(lcd_reset),
		.RS	(initilizer_rs),
		.RW	(initilizer_rw),
		.E		(initilizer_e),
		.DATA	(initilizer_data),
		.done	(initilizer_done)
	);
	
	wire driver_rs, driver_rw, driver_e;
	wire [7:0] driver_data;
	
	LCD_Driver #(
			 .LINES	(ROWS),
			 .CHARS	(COLS),
			 .LINE_STARTS(LINE_STARTS),
			 .use_num_map(1)
		)(
			.clk		(lcd_clk),
			.reset	(lcd_reset),
			.initilized(initilizer_done),
			.RS		(driver_rs),
			.RW		(driver_rw),
			.E			(driver_e),
			.DATA(driver_data),
			//.display_chars(map)
			
			.number_map(map)
		);
	
	assign lcd_rs 		= !initilizer_done ? initilizer_rs 		: driver_rs;
	assign lcd_rw 		= !initilizer_done ? initilizer_rw 		: driver_rw;
	assign lcd_e 		= !initilizer_done ? initilizer_e 		: driver_e;
	assign lcd_data 	= !initilizer_done ? initilizer_data 	: driver_data;
	
endmodule
