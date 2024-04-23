//Zero to 59 second timer 
module SecondsTimer //top level
	(
		input StartStop, clock, reset,
		output [0:6] Aout, Bout
	);
	
	wire [3:0] A, B;
	wire clock_out, one, sec0, sec1;
	wire [3:0] count5, count10; 
	wire [9:0] count1000L, count1000M;
	
	//instantiations
	OneHzClockToggle OneHzClockGenerator
	(
		.TOGGLE(StartStop),
		.CLOCK(clock)  , // input  50-MHz clock
		.RESET(reset) , // input  reset
		.ONEHz(one) // output  OneHz clock k0
	);
	
	divideXn #(4'd10, 3'd4) seconds_low
	(
		.CLOCK(one) , // input  1Hz clock
		.CLEAR(reset) , // input  reset
		.OUT(sec0) , // output  every 10 input pulses
		.COUNT(B) // output [3:0] seconds low count
	);
	
	binary2seven seconds_low_display
	(
		.BIN(B) , // input [3:0] seconds low count
		.SEV(Bout) // output [0:6] low seconds display
	);
	
	divideXn #(3'd6, 2'd3) seconds_high
	(
		.CLOCK(sec0) , // input  from low seconds
		.CLEAR(reset) , // input  CLEAR_sig
		.OUT(sec1) , // output  to low minutes
		.COUNT(A) // output [3:0] high seconds count
	);
	
	binary2seven seconds_high_display
	(
		.BIN(A) , // input [3:0] high seconds count
		.SEV(Aout) // output [0:6] high seconds display
	);
endmodule