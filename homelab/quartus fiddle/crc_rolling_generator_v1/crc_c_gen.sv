/**
continious crc code generator

- polynomial must be known at compile time
- automatically accounts for "+1" on polynomial

- e.g : polynomial x^8 + x^2 + 1
	.N_POLY( (1<<8) ),
	.POLY( 	(1<<2) ) 
*/

module crc_c_gen #(
		parameter N_POLY 	= 'h8,	// order of the polynomial. e.g: for the polynomial "x^7 + ...", N_POLY must be set to 7
		parameter POLY		= 'h2
	)(
		input CLOCK,					// bit is read in from "NEXT_BIT" on the positive edge of each clock
		input RESET,					// active high. sets remainder to the "SEED" value. independent of clock
		input NEXT_BIT,				// next bit to be read in
		input [N_POLY-1:0]	SEED,	// initial value of remainder
		
		output [N_POLY-1:0] 	CRC	// the current CRC value
	);
	
	wire msb = CRC[N_POLY-1];
	
	always_ff @ (posedge CLOCK, posedge RESET) begin
	
		if(RESET == 1) begin
		
			CRC = SEED;		
			
		end else begin
			
			// shift logic
			integer i;
			for(i = N_POLY-1; i > 0; i--)
				if((POLY & (1 << (i-1))) != 0)
					CRC[i] = CRC[i-1] ^ msb;
				else 
					CRC[i] = CRC[i-1];
			
			//edgecase for bit 0.
			if((POLY & 1) != 0)
				CRC[0] = NEXT_BIT ^ msb;
			else
				CRC[0] = NEXT_BIT;
			
		end
	end

endmodule
