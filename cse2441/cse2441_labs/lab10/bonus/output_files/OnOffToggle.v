//OnOff Toggle.  Assumes a normally-on push button switch for OnOff.
module OnOffToggle
	(
		input OnOff,IN,
		output OUT
	);
	
	reg state,nextstate;
	parameter ON=1'b1,OFF=1'b0;
		always @ (negedge OnOff)
			state <= nextstate;
		always @ (state)
			case(state)
				OFF: nextstate = ON; //Pushing OnOff turns the switch on.
				ON: nextstate = OFF; //Pushing OnOff turns the switch off.
			endcase
	assign OUT = state*IN;
endmodule