
module CU(
		input i_TRIG, i_CLOCK, i_CLEAR_ENTRY, i_CLEAR_ALL,
		input [3:0] i_VALUE,
		output reg o_reset, o_loadA, o_loadB, o_addSub, o_loadR, o_IUAU,
		
		//debug
		output reg [2:0] o_state
	);
	reg [2:0] state = 3'b000;
	reg [2:0] nextState = 3'b000;
	parameter 
		S0 = 3'b000,
		S1 = 3'b001,
		S2 = 3'b011,
		S3 = 3'b111,
		S4 = 3'b101;
		
	wire CLK;
	wire [7:0] IU_twosComp;
	
	clock_div#(.WIDTH(64), .DIV(2_000_000)) _clkDiv(
		.clk(i_CLOCK),
		.reset(i_CLEAR_ALL),
		
		.clk_out(CLK)
	);
	
	always o_reset = state != S0;
	always o_loadA = state != S1;
	always o_loadB = state != S3;
	always o_loadR = state != S4;
	always o_IUAU = state == S4;
	
	always o_state = state;
	
	always @ (posedge CLK, negedge i_CLEAR_ALL, negedge i_CLEAR_ENTRY, negedge i_TRIG) begin
		if(i_CLEAR_ALL == 0)begin
			state <= S0;
			nextState <= S0;
		end else if(i_TRIG == 0)
			//state transistions
			case(state)
				S0 : nextState <= S1;
				
				S1 : begin 
						if(i_VALUE == 4'b1010) begin //add
							o_addSub = 0;
							nextState <= S2;
						end else if(i_VALUE == 4'b1011) begin //subtract
							o_addSub = 1;
							nextState <= S2;
						end else 
							nextState <= S1;
					end
				
				S2 : 	nextState <= S3;
				
				S3 : 	if(i_VALUE == 4'b1111)  //add
							nextState <= S4;
						else 
							nextState <= S3;
				
				S4 : 	nextState <= S4;
				
				default : ;
			endcase
		else 
			state <= nextState;
	
	end
	
endmodule