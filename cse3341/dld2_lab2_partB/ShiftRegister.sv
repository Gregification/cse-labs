/**
cse3341, digital logic 2, lab 2
George Boone
1002055713
*/

module ShiftRegister #(
		parameter SIZE = 4,		//size of each element
		parameter NUM 	= 4		//number of elements
	)(
		input 								RESET,		//clears registers
		input									TRIGGER,		//shift trigger
		input logic [SIZE-1:0]			ELE,			//new element to shift in
		output reg	[NUM*SIZE-1:0] 	VALUE			//current value
	);

	always_ff @ (posedge TRIGGER, negedge RESET)
		if(RESET == 1'b0)
			VALUE = 0;
		else if(TRIGGER == 1'b1) begin
			VALUE = VALUE << SIZE;
			VALUE[0+:SIZE] = ELE;
		end

endmodule
