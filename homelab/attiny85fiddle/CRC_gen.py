# INCORRECT GENERATION, DO NOT USE
#   not sure why its putting out hte wrong stuff, also the last element
#   is not correct

import os
import sys

# from wiki : https://en.wikipedia.org/wiki/Cyclic_redundancy_check
def crc_remainder(input_bitstring, polynomial_bitstring, initial_filler):
    """Calculate the CRC remainder of a string of bits using a chosen polynomial.
    initial_filler should be '1' or '0'.
    """
    polynomial_bitstring = polynomial_bitstring.lstrip('0')
    len_input = len(input_bitstring)
    initial_padding = (len(polynomial_bitstring) - 1) * initial_filler
    input_padded_array = list(input_bitstring + initial_padding)
    while '1' in input_padded_array[:len_input]:
        cur_shift = input_padded_array.index('1')
        for i in range(len(polynomial_bitstring)):
            input_padded_array[cur_shift + i] \
            = str(int(polynomial_bitstring[i] != input_padded_array[cur_shift + i]))
    return ''.join(input_padded_array)[len_input:]

def Gen_CRC_Array(start=0, end=256, polynomial_bitstring="{0:b}".format(0x97), initial_filler='0') -> list:
    arr = []

    i = start
    while i <= end:
        arr.append(crc_remainder("{0:b}".format(i),polynomial_bitstring, initial_filler))
        i += 1

    return arr

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python CEC_gen.py <output file>")
        sys.exit(1)
    
    with open(sys.argv[1], "w") as txt_dump:
        txt_dump.write("const uint8_t CRC_table[] = [")
        arr = Gen_CRC_Array()

        i = 0
        for v in arr[:-1]:
            num = int(v, 2)
            txt_dump.write(f" 0x{num:02X},")
            i += 1
            if i % 15 == 0:
                txt_dump.write("\n")

        txt_dump.write(f" 0x{int(arr[-1]):02X}];")
