module OneHzClockToggle (
		input TOGGLE, CLOCK, RESET,
		output ONEHz
	);
	
	wire CLK_toggled;
	
	OnOffToggle toggler(
		.OnOff(TOGGLE),
		.IN(CLOCK),
		.OUT(CLK_toggled)
	);
	
	OneHzClock OneHzClock_inst
	(
		.clock(CLK_toggled) ,	// input  clock_sig
		.reset(RESET) ,	// input  reset_sig
		.OneHz(ONEHz) 	// output  OneHz_sig
	);
	
endmodule