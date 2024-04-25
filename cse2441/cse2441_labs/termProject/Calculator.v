module Calculator(
		input i_CLEAR_ENTRY, i_CLEAR_ALL, i_CLOCK,
		
		input [3:0] i_iu_row,
		
		input i_fancyCycle,
		
		output [0:6] o_HEX0,o_HEX1,o_HEX2,o_HEX3,o_HEX4,o_HEX5,
		output [9:0] o_RLEDS,
		output [3:0] o_iu_col
	);
	
	wire [7:0] iu_twosCompNum;
	wire [3:0] iu_lastDigit;
	wire iu_trig, iu_isValid;
	
	wire 	cu_reset,
			cu_loadA,
			cu_loadB,
			cu_addSub,
			cu_loadR,
			cu_IUAU;	
	
	wire [7:0] au_result;
	
	assign o_RLEDS[9] 	= iu_isValid;
	assign o_RLEDS[8]		= cu_IUAU;
	assign o_RLEDS[7:4] 	= iu_lastDigit;
	
	reg [0:6] hex4;
	always 
		if(cu_loadA == 0)
			hex4 = 7'b0001000;//A
		else if(cu_loadB == 0)
			hex4 = 7'b1100000;//b
		else if(cu_loadR)
			hex4 = 7'b1111010;//r
		else 
			hex4 = 7'b1110110;//=
	assign o_HEX4 = hex4;
	
	assign o_HEX5 = cu_addSub ? 7'b0100100 : 7'b0011000;
	
	IU _iu(
		.CLOCK(i_CLOCK),
		.RESET(i_CLEAR_ENTRY),
		.row(i_iu_row),
		
		.col(o_iu_col),
		.twosComp(iu_twosCompNum),
		.isValid(iu_isValid),
		.o_LAST_BCD(iu_lastDigit),
		.o_trig(iu_trig)
	);
	
	OU _ou0(
		.TC_in(cu_IUAU ? au_result : iu_twosCompNum),
		
		.SEVSEG_SIGN		(o_HEX3),
		.SEVSEG_HUNDREDS	(o_HEX2),
		.SEVSEG_TENS		(o_HEX1),
		.SEVSEG_ONES		(o_HEX0)
	);
	
	CU _cu(
		.i_TRIG(iu_trig),
		.i_CLOCK(i_CLOCK),
		.i_CLEAR_ENTRY(i_CLEAR_ENTRY),
		.i_CLEAR_ALL(i_CLEAR_ALL),
		.i_VALUE(iu_lastDigit),
		
		.o_reset(cu_reset),
		.o_loadA(cu_loadA),
		.o_loadB(cu_loadB),
		.o_addSub(cu_addSub),
		.o_loadR(cu_loadR),
		.o_IUAU(cu_IUAU),
		.o_state(o_RLEDS[3:0])
	);	
	
	AU _au(
		.X(iu_twosCompNum),
		.InA(cu_loadA),
		.InB(cu_loadB),
		.Out(cu_loadR),
		.Clear(i_CLEAR_ALL),
		.Add_Subtract(cu_addSub),
		
		.Result(au_result)
	);
	
	clock_div#(.WIDTH(64), .DIV(50_000_000)) _spinClock(
		.clk(i_CLOCK),
		.reset(i_CLEAR_ALL),
		
		.clk_out(CLK)
	);
	
endmodule