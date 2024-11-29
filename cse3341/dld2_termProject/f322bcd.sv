/**
cse3341, digital logic 2, term project
George Boone
1002055713

float 32 to bcd

- double dabble logic baselined off of 'bin2bcd' module

brute forced, double dabble each side of decimal
*/

module f322bcd
	#(	parameter                 	WIDTH  = 32              , // Bit Width
		parameter                 	DIGITS = 10              , // Digits
		parameter 						RADIX  = 8					 )
	 (	input         [31:0]			bin                      , // ieee754 float32 Input
		output							sign							 ,
		output    reg [3:0]       	bcd         [DIGITS-1:0]); // BCD Output
	
	integer i,j;
	
	reg	[23:0]		eff_mantissa;
	wire 	[23:0]		mantissa;
	wire 	[7:0] 		expo;
	
	assign expo 		= bin[23+:8] - 127;
	assign mantissa 	= bin[0+:23] | (1 << 23);
	assign sign			= bin[31];
	
	always @(bin) begin
		bcd[RADIX] = 4'hF;
		
		//---------------left---------------------
		if(expo >= 0)
			if(expo > 23)
				eff_mantissa = mantissa << (expo - 23);
			else 
				eff_mantissa = mantissa >> (23 - expo);
		else
			eff_mantissa = 0;
		
		for(i = RADIX+1; i < DIGITS; i = i + 1) begin
			bcd[i] = eff_mantissa & (1 << (i-RADIX-1));
		end
		
		//---------------right--------------------
		if(expo < 0)			
			if(expo > -24)
				eff_mantissa = mantissa >> (-expo - 1);
			else
				eff_mantissa = mantissa >> (-expo);
		else if(expo == 0)
			eff_mantissa = mantissa << 1;
		else
			eff_mantissa = 0;
		
		for(i = RADIX-1; i >= 0; i = i - 1) begin
			bcd[i] = eff_mantissa & (1 << (i-RADIX-1));
		end
		
	end

endmodule