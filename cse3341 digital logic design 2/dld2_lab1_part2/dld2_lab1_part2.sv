`define SIZE 32

module dld2_lab1_part2 (
		input COUNT,
		input CLR,
		output Y16, Y20, Y21, Y24
	);
	
	wire [`SIZE-1:0] _value;
	
	assign {Y16, Y20, Y21, Y24} = {_value[16],_value[20],_value[21],_value[24]};
	
	ClockLadderN #(
			.N(`SIZE)
		) __clockLadder (
			.CLK(COUNT),
			.CLR(CLR),
			.VALUE(_value)
		);
	
endmodule