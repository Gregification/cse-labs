`define clear_output \
	begin \
		E = 0; \
		RS = 0; \
		RW = 0; \
		DATA = 8'bzzzzzzzz; \
	end

`define set_output(RS_VAL, RW_VAL, DATA_VAL, E_VAL) \
	begin \
		RS = RS_VAL; \
		RW = RW_VAL; \
		DATA = DATA_VAL; \
		E = E_VAL; \
	end

`define wait(CYCLES, NEXT_STATE) \
	if (counter == CYCLES) next_state = NEXT_STATE;

`define send_command(DATA_VAL, NEXT_STATE) \
	begin \
		if (counter < 32'd100) `set_output(0, 0, DATA_VAL, 0) \
		else if (counter < 32'd900) `set_output(0, 0, DATA_VAL, 1) \
		else if (counter < 32'd1_000) `set_output(0, 0, DATA_VAL, 0) \
		else begin next_state = NEXT_STATE; `clear_output end \
	end

`define set_char(CHAR_VAL, NEXT_STATE) \
	begin \
		if (counter < 32'd100) `set_output(1, 0, CHAR_VAL, 0) \
		else if (counter < 32'd900) `set_output(1, 0, CHAR_VAL, 1) \
		else if (counter < 32'd1_000) `set_output(1, 0, CHAR_VAL, 0) \
		else begin next_state = NEXT_STATE; `clear_output end \
	end

`define check_busy(NEXT_STATE) \
	`wait(32'd12_500, NEXT_STATE)
/* TODO: I am not sure why reading these bidir data lines does not work... I have tried pulldowns and it still does not work. It should not matter.
	begin \
		if (RS == 0 && RW == 1 && E == 1 && DATA[7] == 0) begin \
			next_state = NEXT_STATE; \
			`clear_output \
		end else if (counter == 32'd125) begin \
			if (E == 0) begin \
				`set_output(0, 1, 8'bzzzzzzzz) \
			end else begin \
			`clear_output \
			end \
			counter = 0; \
		end \
	end
*/
