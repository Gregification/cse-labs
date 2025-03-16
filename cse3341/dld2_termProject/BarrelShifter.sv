/**
	cse3341, digital logic 2, term project
	George Boone
	1002055713
	
	- slightly modified version of my lab6
*/

module BarrelShifter#(
		parameter	N 				= 8,
		parameter 	NLayers 		= 3,
		parameter	SHIFT_LEFT 	= 0  //0:right , 1:left
	)(
		input 	[N-1:0] 			IN,
		input 	[NLayers-1:0] 	CTRL,
		output 	[N-1:0] 			OUT
	);
	
	wire [NLayers-1:0][N-1:0] trace;
	
	assign trace[0] 	= IN;
	assign OUT			= trace[NLayers-1];
	
	genvar layer, i, step;
	generate
		for(layer = 1; layer < NLayers; layer += 1) begin : per_layer
			for(i = 0; i < N; i += 1) begin : wire_mux
				localparam step = 2**(layer-1) * (SHIFT_LEFT ? -1 : 1);
				
				Mux2t1 _mux (
						.SELECTOR(CTRL[layer-1]),
						.A(trace[layer-1][i]),
						.B((i+step < N && i+step >= 0) ? trace[layer-1][i + step] : 0),
						
						.SELECTED(trace[layer][i])
					);
					
			end
		end
	endgenerate
	
	
endmodule