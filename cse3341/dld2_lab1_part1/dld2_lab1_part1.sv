//Digital Logic 2. lab 1 part 1
//8x4 register

//this module just routs i/o
//		there are more descriptive comments in the Register_AxB file
module dld2_lab1_part1(
	input [9:0] SWITCHES,
	input CLK, CLR, WrEn,
	
	output[9:0] RLEDS,
	output[6:0] HEX0, HEX5, HEX4
);
	//red leds show switch values
	assign RLEDS = SWITCHES;
	
	//used to send value to seven seg display
	wire [3:0] val;
	
	//the actual 8x4 register
	Register_AxB #(
			.NUM_REGS(8),
			.REG_SIZE(4),
			.ADDR_SIZE(3)
			
		) __register (
			.CLK(CLK),
			.CLR(CLR),
			.WrEn(WrEn),
			.DIN(SWITCHES[0+:4]),
			.DOUT(val),
			.RA(SWITCHES[7+:3]),
			.WA(SWITCHES[4+:3])
		);
	
	//display value of read address
	Bin2seven __bin2seven(
		.BIN(val),
		.SEV(HEX0)
	);
   
	//display read address
	Bin2seven __RA(
		.BIN(SWITCHES[7+:3]),
		.SEV(HEX5)
	);
	
	//display write address
	Bin2seven __WA(
		.BIN(SWITCHES[4+:3]),
		.SEV(HEX4)
	);
	
endmodule