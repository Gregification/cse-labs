module bin2bcd
	#(parameter                 WIDTH  = 32              , // Bit Width
	  parameter                 DIGITS = 10              ) // Digits
	 (input         [WIDTH-1:0] bin                      , // 2's Complement Binary Input
	  output                    sign                     , // Sign Bit
	  output    reg [3:0]       bcd         [DIGITS-1:0]); // BCD Output

	reg [WIDTH-1:0] abs_bin;
	reg [WIDTH+(WIDTH-4)/3:0] packed_bcd;

	assign sign = bin[WIDTH-1];
	
	always @(sign, bin) begin
		abs_bin = (sign == 1) ? -1 * bin : bin;
	end
	
	integer i,j;

	always @(abs_bin) begin
		for(i = 0; i <= WIDTH+(WIDTH-4)/3; i = i+1)
			packed_bcd[i] = 0;
			
		packed_bcd[WIDTH-1:0] = abs_bin;
		
		for(i = 0; i <= WIDTH-4; i = i+1) begin
			for(j = 0; j <= i/3; j = j+1) begin
				if (packed_bcd[WIDTH-i+4*j -: 4] > 4)
					packed_bcd[WIDTH-i+4*j -: 4] = packed_bcd[WIDTH-i+4*j -: 4] + 4'd3;
			end
		end
	end
	
	genvar k;
	generate
		for(k = 0; k < DIGITS; k = k+1) begin : gen_block
			assign bcd[k] = packed_bcd[(k*4)+3 -: 4];
		end	
	endgenerate

endmodule