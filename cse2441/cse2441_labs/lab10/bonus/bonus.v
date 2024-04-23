module bonus(
		input ENABLE_TOGGLE,
		input CLK,
		input RESET,
		output [0:6] SEVSEG_ONES,
		output [0:6] SEVSEG_TENS,
		output [0:6] SEVSEG_HUNDREDS,
		output [0:6] SEVSEG_SIGN
	);
	
	wire CLK_SLOW;
	wire [7:0] testValue;
	
	OneHzClockToggle _ohzct(
		.TOGGLE(ENABLE_TOGGLE),
		.CLOCK(CLK),
		.RESET(RESET),
		.ONEHz(CLK_SLOW)
	);
	
	counter #(.N(8'd256), .M(4'd8)) _c (
        .CLOCK(CLK_SLOW) , 
        .CLEAR(RESET) , 
        .COUNT(testValue) 
    );
	
	OU _ou(
		.TC_in(testValue),
		.SEVSEG_ONES(SEVSEG_ONES),
		.SEVSEG_TENS(SEVSEG_TENS),
		.SEVSEG_HUNDREDS(SEVSEG_HUNDREDS),
		.SEVSEG_SIGN(SEVSEG_SIGN)
	);
	
endmodule