// (NOT TOP LEVEL)
// N-Bit Register
// This module should be instantiated four times in the top level

//////////////////////////////////////////////////////
//
// DO NOT MODIFY THIS FILE
//
//////////////////////////////////////////////////////

module NBitRegister #(parameter N = 4)
(
	input [N-1:0] D,
	input CLK, CLR,
	output reg [N-1:0] Q
);

always @ (posedge CLK, negedge CLR) begin
	if (CLR==1'b0) Q <= 0; //register loaded with all 0’s
	else if (CLK==1'b1) Q <= D; //data input values loaded in register
end

endmodule
