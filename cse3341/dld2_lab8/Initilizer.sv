module LCD_Initilizer
	(input        clk      ,
	 input        reset    ,
	 output       RS, RW, E,
	 inout  [7:0] DATA     ,
	 output reg   done = 0 );

	typedef enum logic [3:0] {
		Wait20ms,
		FunctionSet1,
		Wait5ms,
		FunctionSet2,
		Wait250us,
		FunctionSet3,
		WaitBusy1,
		SetNF,
		WaitBusy2,
		DispOn,
		WaitBusy3,
		EntryMode,
		WaitBusy4,
		DispClear,
		WaitBusy5,
		Done
	} StateType;

	StateType state, next_state;
	reg [31:0] counter = 0;

	always @(posedge clk or posedge reset) begin
		if (reset) begin
			state = Wait20ms;
			next_state = Wait20ms;
			done = 0;
			counter = 0;
		end else begin
			if (state != next_state) begin
				state = next_state;
				counter = 0; // Reset counter
			end else
				counter = counter + 1; // Increment counter

			case (state)
				Wait20ms: `wait(32'd1_000_000, FunctionSet1) // 20ms
				FunctionSet1: `send_command(8'b00110000, Wait5ms) // Func Set Pulse
				Wait5ms: `wait(32'd250_000, FunctionSet2) // 5ms
				FunctionSet2: `send_command(8'b00110000, Wait250us) // Func Set Pulse
				Wait250us: `wait(32'd12_500, FunctionSet3) // 250us
				FunctionSet3: `send_command(8'b00110000, WaitBusy1) // Func Set Pulse
				WaitBusy1: `check_busy(SetNF) // Wait for BF
				SetNF: `send_command(8'b00111000, WaitBusy2) // Func Set 2-Lines | 5 X 8 Char Mode
				WaitBusy2: `check_busy(DispOn) // Wait for BF
				DispOn: `send_command(8'b00001100, WaitBusy3) // Display On | Cursor Off
				WaitBusy3: `check_busy(EntryMode) // Wait for BF
				EntryMode: `send_command(8'b00000110, WaitBusy4) // Entry Mode Increment | No Display Shift
				WaitBusy4: `check_busy(DispClear) // Wait for BF
				DispClear: `send_command(8'b00000001, WaitBusy5) // Display Clear
				WaitBusy5: `check_busy(Done) // Wait for BF
				Done: done = 1; // Set done
				default: next_state = Wait20ms;
			endcase
		end
	end
endmodule