/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

/**
	sores values from the KeyPad into a shift register.
	requires a external shift signal to tell when to store the selected value.
	
	- acts as 'keypad_fsm' and 'keypad_decoder' from "Class 2.1 fall 2024" slides(35,38,39) in one.
	
	note:
	- requires well timed external clock signal for scanning, no clock speed control is done in this module.
	- key combos not supported
		- last row last column gets priority
*/
module KeyPad #(
		parameter N = 4			//number of values to hold in the shift register
	)(
		input 	[3:0]	ROWS,		//row
		
		input 			CLK,		//clock rate used in scanning
		input				SHIFT,	//shift in the next value. this could be internal but external lets you do things like double tap
		input				RESET,	//reset shift register
		
		output reg [3:0]	COLS,		//column triggers
		output reg [3:0]	NXTVAL,	//next value to be shifted in
		output reg [15:0] VALUE,	//current value
		output reg 			PRESSED 	//if the keypad is currently pressed (tied to keys refresh cycle - aka, needs clock to work)
	);
	
	ShiftRegister #(
				.SIZE(4),
				.NUM(N)
		) _shift_register (
				.RESET(RESET),
				.TRIGGER(SHIFT),
				.ELE(NXTVAL),
				.VALUE(VALUE)
		);
	
	//using registers because the value range makes itterating easier
	reg [1:0] c; 		//column itterator
	reg [1:0] pc, pr;	//column & row of the last pressed value (aka : next value)
	
	integer i;			//temp values
	
	/**
		itterates 1 column each tick, if the element is selected then thats the current value.
		- note that scanning is indipendent of resetting
		
		[start]
		[tick]
		1. check all rows, assume any match is caused by current column.
		2. disable current column, enable next column.
		[tick]
		#repeat
		
	*/
	always_ff @ (posedge CLK, negedge RESET) 
		if(RESET == 1'b0) begin
			pr 		<= 0;
			pc 		<= 0;
			PRESSED 	<= 1'b0;
			NXTVAL	<= 4'd0;
		end
		else if(CLK == 1'b1) begin
			
			//check rows
			for(i = 0; i < 4; i = i + 1)
				//if row shorted => is pressed
				if(ROWS[i] == 1'b0) begin
					pc = c;
					pr = i;
					
					NXTVAL = pr * 4 + pc;
				end
			
			//update pressed status
			if(pc == c)
				PRESSED <= ROWS[pr] == 1'b0;
			
			//itterate column
			c = c + 1'b1;
			COLS = 1 << c;
		end
	
endmodule
