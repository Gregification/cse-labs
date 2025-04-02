/**
cse3341, digital logic 2, lab 4
George Boone
1002055713

modified version of the one hot controller shown in class 3.2 (slide 48)

hardcoded for the signed 4x4 shift add multiplier
*/

module ControlUnit(
		input CLK, RESET,
		input Q0, Qs,		// last Q and last shifted out of Q
		
		output HALT, START, TEST, ADD, SUBTRACT, SHIFT
	);
	reg [5:0] _state;
	reg [1:0] _counter;
	
	assign {HALT, START, TEST, ADD, SUBTRACT, SHIFT} = _state;
	parameter 
		_state_halt			= 6'b100000, //0x20
		_state_start		= 6'b010000, //0x10
		_state_test			= 6'b001000, //0x08
		_state_add			= 6'b000100, //0x04
		_state_subtract	= 6'b000010, //0x02
		_state_shift		= 6'b000001; //0x01
	
	
	always @(posedge CLK, negedge RESET)
		if(RESET == 0) begin
			_state <= _state_start;
			_counter <= 0;
		end else if(CLK == 1)
			case (_state)
				_state_halt: 		_state <= _state_halt;
				_state_start:		_state <= _state_test;
				_state_test:	
						case({Q0,Qs})
							2'b01: 	_state <= _state_add;
							2'b10:	_state <= _state_subtract;
							default: _state <= _state_shift;
						endcase
				_state_add:			_state <= _state_shift;
				_state_subtract: 	_state <= _state_shift;
				_state_shift:		begin
						if(_counter[0] & _counter[1])
							_state 	<= _state_halt;
						else begin
							_state 	<= _state_test;
							_counter <= _counter + 1;							
						end
					end
			endcase
			
	
endmodule
