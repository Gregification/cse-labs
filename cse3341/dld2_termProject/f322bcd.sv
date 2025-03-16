/**
cse3341, digital logic 2, term project
George Boone
1002055713

float 32 to bcd
*/

module f322bcd
	#(	parameter                 	WIDTH  = 32              , // Bit Width
		parameter                 	DIGITS = 10              , // Digits
		parameter 						RADIX  = 8					 )
	 (	input         [31:0]			bin                      , // ieee754 float32 Input
		output							sign							 ,
		output    reg [3:0]       	bcd         [DIGITS-1:0]); // BCD Output
	
	integer i,j;
	
	reg [11:0] packed_bcd;
	
	reg	[23:0]		eff_mantissa;
	wire 	[23:0]		mantissa;
	wire 	[7:0] 		expo;
	
	assign expo 		= bin[23+:8];
	assign mantissa 	= bin[0+:23] | (1 << 23);
	assign sign			= bin[31];
	
	always @(bin) begin
		bcd[RADIX] = 4'hF;
		
		//---------------left---------------------
		if(expo >= 127)
			if(expo > 127+23)
				eff_mantissa = mantissa << (expo - (127+23));
			else 
				eff_mantissa = mantissa >> ((127+23) - expo);
		else
			eff_mantissa = 0;
		
		for(i = RADIX+1; i < DIGITS; i = i + 1) begin
			bcd[i] = (eff_mantissa & (1 << (i-RADIX-1))) != 0;
		end
		
		//---------------right--------------------
		if(expo < 127)
			eff_mantissa = mantissa >> (127-expo - 1);
		else if(expo > 127)
			eff_mantissa = mantissa << (expo-127 + 1);
		else
			eff_mantissa = mantissa << 1;
			
		
		for(i = RADIX-1; i >= 0; i = i - 1) begin
			bcd[i] = (eff_mantissa & (1 << (23 - ((RADIX-1)-i)))) != 0;
		end
		
		//-------------exponent-------------------
		for(i = 0; i <= 8+(8-4)/3; i = i+1)
			packed_bcd[i] = 0;
			
		packed_bcd[8-1:0] = expo;
		
		for(i = 0; i <= 8-4; i = i+1) begin
			for(j = 0; j <= i/3; j = j+1) begin
				if (packed_bcd[8-i+4*j -: 4] > 4)
					packed_bcd[8-i+4*j -: 4] = packed_bcd[8-i+4*j -: 4] + 4'd3;
			end
		end
		
		bcd[16]	= packed_bcd[3 -:4];
		bcd[17]	= packed_bcd[7 -:4];
		bcd[18]	= packed_bcd[11-:4];
		
	end

endmodule