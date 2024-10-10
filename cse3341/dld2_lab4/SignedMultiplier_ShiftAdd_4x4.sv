/**
cse3341, digital logic 2, lab 4
George Boone
1002055713

based off of figure 1 of the assignment
	- no register modules because of difficulity in synching loads form the adder
	- hardcoded 4 bits each input, much change control counter logic if N bits changed
*/

`define N 4

module SignedMultiplier_ShiftAdd_4x4(
		input 		CLK, RESET,
		input	wire	[`N-1:0]	PLICAND, PLIER,	// multiplicand, multiplier
		
		output DONE,
		output reg [(`N*2)-1:0]	PRODUCT,
		
		// debug
		input InM,InQ,
		output wire [`N-1:0] A, Q,
		output reg [`N-1:0]	M,
		output reg [5:0] 		STATE,
		output reg _Q_shift_out	// register shift outputs
	);
	
	// wiring
	reg _carryflag;
	wire [`N:0] _adder_o, _subtractor_o;	   // add/sub outputs
	wire _cout;											// adder
	wire _sHalt, _sStart, _sTest, _sAdd, _sSubtract, _sShift;
	
	// read only values of product
	assign A = PRODUCT[4+:4];
	assign Q = PRODUCT[0+:4];
	
	// debug
	assign STATE = {_sHalt, _sStart, _sTest, _sAdd, _sSubtract, _sShift};
	
	// ----------------------------------------------------------------------
	// state logic
	// ----------------------------------------------------------------------
	
	assign DONE = STATE[5] == 1;
	
	always @ (posedge CLK, posedge InQ, posedge InM) begin
		if(InQ == 1) begin
			PRODUCT[0+:4] = PLIER;	// assign Q
		end else
		if(InM == 1) begin
			M = PLICAND;
		end else
		
		if(_sStart == 1) begin
			PRODUCT[4+:4] = 0; // zero A
			_Q_shift_out = 0;
		end
		
		else if(_sTest == 1) begin
			// do nothing
		end
		
		else if(_sShift == 1) begin
			_Q_shift_out = PRODUCT[0];
			
			// dosent actually sign extend ?!?!?!?!?!?
			// i hate verilog
			PRODUCT = PRODUCT >>> 1;
		end
		
		else if(_sAdd == 1) begin
			PRODUCT[4+:4] = _adder_o;			// assign A
		end
		
		else if(_sSubtract == 1) begin
			PRODUCT[4+:4] = _subtractor_o;	// assign A
		end
		
	end
	
//	always @ (posedge _sTest) begin
//		// do nothing
//	end
		
	
	// ----------------------------------------------------------------------
	// modules
	// ----------------------------------------------------------------------	
	
	// adder
	Adder #(
			.N(`N)
		) __adder (
			.A(A),
			.B(M),
			
			.S(_adder_o)
		);
	
	// subtractor
	Subtractor #(
			.N(`N)
		) __subtractor (
			.A(A),
			.B(M),
			
			.S(_subtractor_o)
		);
		
	// control unit
	ControlUnit __control_unit(
			.CLK(CLK),
			.RESET(RESET),
			.Q0(Q[0]),
			.Qs(_Q_shift_out),
			
			.HALT(_sHalt),
			.START(_sStart), 
			.TEST(_sTest),
			.ADD(_sAdd), 
			.SUBTRACT(_sSubtract), 
			.SHIFT(_sShift)
		);
	
endmodule
