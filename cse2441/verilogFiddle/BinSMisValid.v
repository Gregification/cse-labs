//checks if signed magnitude is within [MIN, MAX] inclusive
module BinSMisValid#(parameter BITS = 8, parameter MIN = -127, parameter MAX = 127) (
        input [BITS-1:0] signedMag,
        output valid,
        output inv_valid
    );

    assign inv_valid = signedMag < MIN || signedMag > MAX;
    assign valid = ~inv_valid;

endmodule