/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713
	
	shift tester
*/

module ShiftTester(
		input OnOff, Clear, Set, Clock,
		output [2:0] B,
		
		// debug
		output wire onoff_internal
	);
	
	OnOffToggle(
		.TOGGLE(OnOff),
		
		.ON(onoff_internal)
	);
	
	BinCounter #(
			.N(2)
		) _bc (
			.COUNT(Clock & onoff_internal),
			.RESET(Clear),
			.SET(Set),
			
			.VALUE(B)
		);
	
endmodule
