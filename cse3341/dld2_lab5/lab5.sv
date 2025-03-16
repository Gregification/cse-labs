/**
	cse3341, digital logic 2, lab 5
	George Boone
	1002055713

	modular exponent subtractor
	this module is not part of the logic, it only serves to demo it
*/

`define N 8

module lab5(
		// standard in. i got tired of the assignment editor, see immideatly after wire decelerations for assignments
		input [9:0] DEVBOARD_SWS,
		input [1:0] DEVBOARD_BTN,
		output[9:0] DEVBOARD_RLEDS,	
		output[47:0]DEVBOARD_SEGS
	);
	
	
//------------------------------------------------------------------------------
// 	INPUT ASSIGMENTS  \ EXTERNAL WIRING
//------------------------------------------------------------------------------
	
	wire [`N-1:0] val_in;
	wire ina, inb;
	wire cout;
	reg showAelseB, showResult;	//yeah
	
	assign val_in		= DEVBOARD_SWS[0+:`N];
	assign ina			= DEVBOARD_BTN[0];	
	assign inb			= DEVBOARD_BTN[1];
	assign showAelseB = DEVBOARD_SWS[8];
	assign showResult = DEVBOARD_SWS[9];
	
	assign DEVBOARD_RLEDS[9]	= cout;
	
	//blank the seg displays
	assign DEVBOARD_SEGS = ~0;
	
//------------------------------------------------------------------------------
// 	INTERNAL WIRING
//------------------------------------------------------------------------------	
	
	reg [`N:0] valA, valB, diff;
	
	always @ (posedge ina)
		valA = val_in;
	
	always @ (posedge inb)
		valB = val_in;
		
	always @ (showAelseB, showResult)
		if(showResult == 1)
			DEVBOARD_RLEDS[0+:`N] = diff;
		else if(showAelseB == 1)
			DEVBOARD_RLEDS[0+:`N] = valA;
		else
			DEVBOARD_RLEDS[0+:`N] = valB;
		
//------------------------------------------------------------------------------
// 	MODULES
//------------------------------------------------------------------------------	

//there is a bug where valA and/or valB are updated when flicking around the switches.
// i 

//works
//	RCSubtractor(
//			.A(valA),
//			.B(valB),
//			
//			.D(diff),
//			.COUT(cout)
//		);

//works
//	BintTwosComp(
//			.BIN(valA),
//			
//			.TWOSCOMP(diff),
//			.COUT(cout)
//		);

	ExponentSubtractor#(
			.N(`N)
		)(
			.EA(valA),
			.EB(valB),

			.Cout(cout),
			.ED(diff)
		);
		
endmodule
