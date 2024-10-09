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
		output wire [`N*2-1:0]	PRODUCT,
		
		// debug
		input InM,InQ,
		output reg [`N-1:0] Q, M,
		output reg [`N:0]	 A,
		output reg [5:0] STATE
	);
	
	// wiring
	reg _A_shift_out, _Q_shift_out;				// register shift outputs
	wire [`N:0] _adder_o, _subtractor_o;	   // add/sub outputs
	wire _cout;											// adder
	wire _sHalt, _sStart, _sTest, _sAdd, _sSubtract, _sShift;
	
	// product
	assign PRODUCT = {A[0+:`N], Q};
	
	// debug
	assign STATE = {_sHalt, _sStart, _sTest, _sAdd, _sSubtract, _sShift};
	
	// ----------------------------------------------------------------------
	// state logic
	// ----------------------------------------------------------------------
	
	always @ (posedge _sHalt) begin
		DONE <= 1;
	end
	
	always @ (posedge _sStart, posedge _sShift, posedge _sAdd, posedge _sSubtract, posedge InQ, posedge InM) begin
		if(InQ == 1) begin
			Q <= PLIER;
		end
		
		else if(InM == 1) begin
			M <= PLICAND;
		end
	
		if(_sStart == 1) begin
			Q <= PLIER;
			M <= PLICAND;
			A <= 0;
		end
		
		else if(_sShift == 1) begin
			_A_shift_out <= A[0];
			_Q_shift_out <= Q[0];
			
			Q <= Q >> 1;		
			A <= A >> 1;
			
			Q[`N-1] 	<= _A_shift_out;
		end
		
		else if(_sAdd == 1) begin
			A <= _adder_o; 		// this includes the carry
		end
		
		else if(_sSubtract == 1) begin
			A <= _subtractor_o; 	// this includes the carry
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
