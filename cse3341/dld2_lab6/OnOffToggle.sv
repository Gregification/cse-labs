/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713
*/

module OnOffToggle(
		input TOGGLE,
		
		output ON
	);
	
	always_ff @ (posedge TOGGLE)
		ON = ~ON;
	
endmodule
