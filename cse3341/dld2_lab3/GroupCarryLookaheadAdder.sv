/**
cse3341, digital logic 2, lab 3
George Boone
1002055713

wiring done using general formula of C_1 = G_0 | ( P_0 & C_0 )

other things
	- hardcoded equations since I couldnt get bool equations to expand automatically
	- hardcoded valuses because I hard coded the equations
	- looked into verilog 'funcitons' and 'generate loops' but neither seem to expand boolean algebra
*/

module GroupCarryLookaheadAdder(
		input [7:0] A, B,
		input ADD_SUBTRACT,
	
		output [7:0] R,
		output logic [8:0] C		//carry in values
	);
	
	
	logic [7:0] g, p, s;
	
	//setup adders and wire the carries, g, and p
	generate
		genvar i;
		for(i = 0; i < 8; i = i + 1) begin : bb
			FullAdder __full_adder (
					.A(A[i]),
					.B(B[i] ^ ADD_SUBTRACT),
					.Cin(i == 0 ? ADD_SUBTRACT : C[i]),
					
					.S(R[i]),
					.G(g[i]),
					.P(p[i])
				);
		end
	endgenerate
	
	//manually setup C[0]
	// exact same value as the first adder module but it gets messy if i try to take it form there
	//assign C[0] = A[0] & (B[0] ^ ADD_SUBTRACT);
	
	
	//brute force it, see comments at top of this file for my reasoning
	//carry lookahead equation for carries
	//	equations were generated with some custom java (see end of file)
	assign C[1] = (g[0]) | (C[0] & p[0]);
	assign C[2] = (g[0] & p[1]) | (C[0] & p[0] & p[1]) | (g[1]);
	assign C[3] = (g[0] & p[1] & p[2]) | (C[0] & p[0] & p[1] & p[2]) | (g[1] & p[2]) | (g[2]);
	assign C[4] = (g[0] & p[1] & p[2] & p[3]) | (C[0] & p[0] & p[1] & p[2] & p[3]) | (g[1] & p[2] & p[3]) | (g[2] & p[3]) | (g[3]);
	assign C[5] = (g[0] & p[1] & p[2] & p[3] & p[4]) | (C[0] & p[0] & p[1] & p[2] & p[3] & p[4]) | (g[1] & p[2] & p[3] & p[4]) | (g[2] & p[3] & p[4]) | (g[3] & p[4]) | (g[4]);
	assign C[6] = (g[0] & p[1] & p[2] & p[3] & p[4] & p[5]) | (C[0] & p[0] & p[1] & p[2] & p[3] & p[4] & p[5]) | (g[1] & p[2] & p[3] & p[4] & p[5]) | (g[2] & p[3] & p[4] & p[5]) | (g[3] & p[4] & p[5]) | (g[4] & p[5]) | (g[5]);
	assign C[7] = (g[0] & p[1] & p[2] & p[3] & p[4] & p[5] & p[6]) | (C[0] & p[0] & p[1] & p[2] & p[3] & p[4] & p[5] & p[6]) | (g[1] & p[2] & p[3] & p[4] & p[5] & p[6]) | (g[2] & p[3] & p[4] & p[5] & p[6]) | (g[3] & p[4] & p[5] & p[6]) | (g[4] & p[5] & p[6]) | (g[5] & p[6]) | (g[6]);
	assign C[8] = (g[0] & p[1] & p[2] & p[3] & p[4] & p[5] & p[6] & p[7]) | (C[0] & p[0] & p[1] & p[2] & p[3] & p[4] & p[5] & p[6] & p[7]) | (g[1] & p[2] & p[3] & p[4] & p[5] & p[6] & p[7]) | (g[2] & p[3] & p[4] & p[5] & p[6] & p[7]) | (g[3] & p[4] & p[5] & p[6] & p[7]) | (g[4] & p[5] & p[6] & p[7]) | (g[5] & p[6] & p[7]) | (g[6] & p[7]) | (g[7]);
	
endmodule


//java used to generate the cla equations
/**
    public static void main(String[] args) {
        ArrayList<String> s = new ArrayList<String>(Arrays.asList("g[0]","C[0] & p[0]"));

        for(int i = 1; i < 9; i++){
            c(i, s);
            System.out.print("C[" + (i) + "] = ");

            var sb = new StringBuilder();
            for(var v : s)
                sb.append("(" + v + ")" + " | ");
            sb.setLength(sb.length() - 3);
            
            System.out.println(sb.toString());
        }
    }

    public static void c(int n, ArrayList<String> ret){
        if(n == 1) 
            return;

        for(int i = 0; i < ret.size(); i++)
            ret.set(i, ret.get(i) + " & p[" + (n-1) + "]");

        ret.add("g[" + (n-1) + "]");
    }
*/
