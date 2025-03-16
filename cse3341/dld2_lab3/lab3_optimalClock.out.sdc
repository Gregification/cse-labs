## Generated SDC file "lab3_optimalClock.out.sdc"

## Copyright (C) 2020  Intel Corporation. All rights reserved.
## Your use of Intel Corporation's design tools, logic functions 
## and other software and tools, and any partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Intel Program License 
## Subscription Agreement, the Intel Quartus Prime License Agreement,
## the Intel FPGA IP License Agreement, or other applicable license
## agreement, including, without limitation, that your use is for
## the sole purpose of programming logic devices manufactured by
## Intel and sold by Intel or its authorized distributors.  Please
## refer to the applicable agreement for further details, at
## https://fpgasoftware.intel.com/eula.


## VENDOR  "Altera"
## PROGRAM "Quartus Prime"
## VERSION "Version 20.1.1 Build 720 11/11/2020 SJ Lite Edition"

## DATE    "Fri Sep 27 11:17:47 2024"

##
## DEVICE  "10M50DAF484C7G"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {INB} -period 1.000 -waveform { 0.000 0.500 } [get_ports {INB}]
create_clock -name {INA} -period 1.000 -waveform { 0.000 0.500 } [get_ports {INA}]
create_clock -name {CLK} -period 3.753 -waveform { 0.000 1.876 } [get_ports {CLK}]
create_clock -name {OUT} -period 1.000 -waveform { 0.000 0.500 } [get_ports {OUT}]
create_clock -name {ClockLadderN:__clkladder|VALUE[17]} -period 1.000 -waveform { 0.000 0.500 } [get_registers {ClockLadderN:__clkladder|VALUE[17]}]
create_clock -name {HexBoard:__hexboard|CAT[0]} -period 1.000 -waveform { 0.000 0.500 } [get_registers {HexBoard:__hexboard|CAT[0]}]


#**************************************************************
# Create Generated Clock
#**************************************************************



#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************

set_clock_uncertainty -rise_from [get_clocks {INB}] -rise_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {INB}] -fall_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {INB}] -rise_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {INB}] -fall_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {INA}] -rise_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {INA}] -fall_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {INA}] -rise_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {INA}] -fall_to [get_clocks {OUT}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {OUT}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {OUT}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {OUT}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.030  
set_clock_uncertainty -fall_from [get_clocks {OUT}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.030  
set_clock_uncertainty -rise_from [get_clocks {CLK}] -rise_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {CLK}] -fall_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {CLK}] -rise_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {CLK}] -fall_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {CLK}] -rise_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {CLK}] -fall_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {CLK}] -rise_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {CLK}] -fall_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -rise_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -fall_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -rise_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -fall_to [get_clocks {CLK}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {ClockLadderN:__clkladder|VALUE[17]}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -rise_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -fall_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -rise_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -fall_to [get_clocks {ClockLadderN:__clkladder|VALUE[17]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -rise_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {HexBoard:__hexboard|CAT[0]}] -fall_to [get_clocks {HexBoard:__hexboard|CAT[0]}]  0.020  


#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************



#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************



#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************

