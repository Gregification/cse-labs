/**
	cse3341, digital logic 2, lab 6
	George Boone
	1002055713
	
	hardcoded 8 bit barrel shifter
	
	- variable naming as shown in figure 1 of lab document
*/

module BarrelShifter_8r(
		input [7:0] A,
		input [2:0] B,
		output [7:0] Y
	);
	
	wire [7:0] b2t1, b1t0; // wires between each layer, cant figure out verilogs packed array stuff
	
	genvar layer, i;
	generate
		for(layer = 0; layer < 3; layer = layer+1) begin : per_layer
			for(i = 0; i < 8; i = i+1) begin : wire_mux
			
				localparam f = i + (1 << layer); //index to shift in from
				
				Mux2t1 _mux (
						.SELECTOR(B[layer]),
						.A((layer==0) ? b1t0[i] : (layer==1) ? b2t1[i] : A[i]),
						.B((f>7) ? 0 : (layer==0) ? b1t0[f] : (layer==1) ? b2t1[f] : A[f]),
						
						.SELECTED((layer==0) ? Y[i] : (layer==1) ? b1t0[i] : b2t1[i])
					);
				
			end
		end
	endgenerate
	
	
endmodule
