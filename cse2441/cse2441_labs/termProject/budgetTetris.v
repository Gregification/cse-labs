module budgetTetris#(
		parameter NLAYERS = 6
	)(
		input clk,	//cycle clock
		input reset,
		
		input rot,
		
		output signal,
		output [0:6] o_HEX0,
		output [0:6] o_HEX1,
		output [0:6] o_HEX2,
		output [0:6] o_HEX3,
		output [0:6] o_HEX4,
		output [0:6] o_HEX5
	);
	parameter 
	 	_0 = 7'b0000001,
        _1 = 7'b0000010,
        _2 = 7'b0000100,
        _3 = 7'b0001000,
        _4 = 7'b0010000,
        _5 = 7'b0100000,
        _6 = 7'b1000000;
	integer i, j, fl, fs;
	integer ctrlL, ctrlS, ctrlR;

	reg [0:7] insrt;
	reg [6:0] hex [0:6];

	assign signal = reset == 0;
	assign o_HEX0 = hex[5];
	assign o_HEX1 = hex[4];
	assign o_HEX2 = hex[3];
	assign o_HEX3 = hex[2];
	assign o_HEX4 = hex[1];
	assign o_HEX5 = hex[0];

	always @ (negedge clk, negedge reset) begin		
		if(reset == 0) begin
			for(i = 0; i < NLAYERS; i = i + 1) begin
				if(i == 3) hex[i] <= ~(_3 | _6 | _1 | _4);
				else hex[i] <= ~(7'b0);
			end

			ctrlL = 0;
			ctrlS = 0;
		end else if(rot == 1) begin
			ctrlR = ctrlR + 1;
			if(ctrlR > 6)
				ctrlR = 0;

			if((~hex[ctrlL]&(1<<ctrlR)) == 0) begin
				hex[ctrlL][ctrlS] <= 1;
				ctrlS <= ctrlR;
				hex[ctrlL][ctrlR] <= 0;
			end

		end else begin //cycle
			for(i = NLAYERS-1; i >= 0; i = i - 1) begin
				if((~hex[i]&_5) != 0 && (~hex[i]&_4) != 0)
					hex[i] <= (hex[i] | (_5 | _4));

				if((~hex[i]&_1) != 0 && (~hex[i]&_2) != 0)
					hex[i] <= (hex[i] | (_1 | _2));
				
				if((~hex[i]&_6) != 0 && (~hex[i]&_0) != 0 && (~hex[i]&_3) != 0)
					hex[i] <= (hex[i] | (_6 | _0 | _3));

				for(j = 6; j >= 0; j = j - 1) if(i != 0 || (j == 4 || j == 5)) begin
					fl = i - 1; fs = j; //from layer, from segment
					case(j)
						1 : begin
							fs = 5;
						end
						2 : begin
							fs = 4;
						end
						4 : begin
							fl = i;
							fs = 2;
						end
						5 : begin
							fl = i;
							fs = 1;
						end
						default : ;
					endcase

					// if [to] is empty : move [from] -> [to]
					if((~hex[i]&(1<<j)) == 0) begin
						hex[i][j] <= (~hex[fl]&(1<<fs)) == 0;

						//deactivate [from]
						hex[fl][fs] <= 1;

						if(fl == ctrlL && fs == ctrlS) begin //if is control, update controlled
							ctrlL = i;
							ctrlS = j;
						end
					end else if(i == ctrlL && j == ctrlS) begin // if is control, make new one
						ctrlL = 0;
						ctrlS = 0;
					end
				end
			end
			
			// 2 left vert bars arnt covered by the loop, but must be deactivated
			hex[0][1] <= 1;
			hex[0][2] <= 1;

			if((~hex[ctrlL]&(1<<ctrlS)) == 0)
				hex[ctrlL][ctrlS] <= 0;
		end
	end
	
endmodule