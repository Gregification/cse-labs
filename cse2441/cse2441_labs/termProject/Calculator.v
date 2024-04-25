module Calculator(
		input i_CLEAR_ENTRY, i_CLEAR_ALL, i_CLOCK,
		
		input [3:0] i_iu_row,
		
		input i_game,
		
		output [0:6] o_HEX0,o_HEX1,o_HEX2,o_HEX3,o_HEX4,o_HEX5,
		output [9:0] o_RLEDS,
		output [3:0] o_iu_col
	);
	
	wire  [6:0] _HEX_calculator [0:6];//5 elments of 7 bits each
	reg  [6:0] _HEX_game [0:6];
	
	assign o_HEX0 = i_game ? _HEX_game[0] : _HEX_calculator[0];
	assign o_HEX1 = i_game ? _HEX_game[1] : _HEX_calculator[1];
	assign o_HEX2 = i_game ? _HEX_game[2] : _HEX_calculator[2];
	assign o_HEX3 = i_game ? _HEX_game[3] : _HEX_calculator[3];
	assign o_HEX4 = i_game ? _HEX_game[4] : _HEX_calculator[4];
	assign o_HEX5 = i_game ? _HEX_game[5] : _HEX_calculator[5];
	
	wire 	[7:0] iu_twosCompNum;
	wire 	[3:0] iu_lastDigit;
	wire 	iu_trig, iu_isValid;
	
	wire 	cu_reset,
			cu_loadA,
			cu_loadB,
			cu_addSub,
			cu_loadR,
			cu_IUAU;	
	
	wire [7:0] au_result;
			
	assign o_RLEDS[9] 	= iu_isValid;
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
	assign _HEX_calculator[4] = hex4;
	
	assign _HEX_calculator[5] = cu_addSub ? 7'b0100100 : 7'b0011000;
	
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
		
		.SEVSEG_SIGN		(_HEX_calculator[3]),
		.SEVSEG_HUNDREDS	(_HEX_calculator[2]),
		.SEVSEG_TENS		(_HEX_calculator[1]),
		.SEVSEG_ONES		(_HEX_calculator[0])
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
	
	//game stuff	
	wire _game_clock;
	reg gameReset;
	
	always @ (negedge i_game) 
		gameReset = ~gameReset;
	
	assign o_RLEDS[8] = _game_clock;
	
	working_clock_div#(.WIDTH(64), .DIV(50000000)) _spinClock(
		.clk(i_CLOCK),
		.reset(i_CLEAR_ALL),
		
		.clk_out(_game_clock)
	);
	
	budgetTetris _bt(
		.clk(_game_clock || iu_lastDigit == 4'hC),
		.reset(i_game),
		.rot(iu_lastDigit == 4'hD),
		
		.signal(o_RLEDS[3]),
		.o_HEX0(_HEX_game[0]),
		.o_HEX1(_HEX_game[1]),
		.o_HEX2(_HEX_game[2]),
		.o_HEX3(_HEX_game[3]),
		.o_HEX4(_HEX_game[4]),
		.o_HEX5(_HEX_game[5])
	);
	
endmodule