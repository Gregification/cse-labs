// (NOT TOP LEVEL)
// Full Adder Behavioral
// This module should NOT be instantiated in the top level
// This module should only be instantiated in RippleCarryAdderStructural.v

//////////////////////////////////////////////////////
//
// DO NOT MODIFY THIS FILE
//
//////////////////////////////////////////////////////

module FAbehav 
(
	input ai, bi, cini,
	output reg si, couti
);

always @ (ai,bi,cini)
	case ({ai,bi,cini})
		3'b000: begin couti = 1'b0; si = 1'b0; end
		3'b001: begin couti = 1'b0; si = 1'b1; end
		3'b010: begin couti = 1'b0; si = 1'b1; end
		3'b011: begin couti = 1'b1; si = 1'b0; end
		3'b100: begin couti = 1'b0; si = 1'b1; end
		3'b101: begin couti = 1'b1; si = 1'b0; end
		3'b110: begin couti = 1'b1; si = 1'b0; end
		3'b111: begin couti = 1'b1; si = 1'b1; end
	endcase
endmodule
