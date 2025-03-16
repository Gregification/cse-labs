# Xilinx XUP Blackboard rev D Pinning (blackboard.xdc)
# Jason Losh

# Clock
set_property PACKAGE_PIN H16 [get_ports CLK100]; 
set_property IOSTANDARD LVCMOS33 [get_ports CLK100];
create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 4} [get_ports CLK100];

# LEDs (green for 0-9
set_property PACKAGE_PIN N20 [get_ports LED[0]]; 
set_property PACKAGE_PIN P20 [get_ports LED[1]]; 
set_property PACKAGE_PIN R19 [get_ports LED[2]]; 
set_property PACKAGE_PIN T20 [get_ports LED[3]]; 
set_property PACKAGE_PIN T19 [get_ports LED[4]]; 
set_property PACKAGE_PIN U13 [get_ports LED[5]]; 
set_property PACKAGE_PIN V20 [get_ports LED[6]]; 
set_property PACKAGE_PIN W20 [get_ports LED[7]]; 
set_property PACKAGE_PIN W19 [get_ports LED[8]]; 
set_property PACKAGE_PIN Y19 [get_ports LED[9]]; 
set_property IOSTANDARD LVCMOS33 [get_ports LED[*]];

# RGB LEDs
set_property PACKAGE_PIN W18 [get_ports RGB0[0]]; 
set_property PACKAGE_PIN W16 [get_ports RGB0[1]]; 
set_property PACKAGE_PIN Y18 [get_ports RGB0[2]];
set_property IOSTANDARD LVCMOS33 [get_ports RGB0[*]];
set_property PACKAGE_PIN Y14 [get_ports RGB1[0]]; 
set_property PACKAGE_PIN Y16 [get_ports RGB1[1]]; 
set_property PACKAGE_PIN Y17 [get_ports RGB1[2]]; 
set_property IOSTANDARD LVCMOS33 [get_ports RGB1[*]];

# 7-segment displays (with dp)
set_property PACKAGE_PIN K19 [get_ports SS_ANODE[0]];
set_property PACKAGE_PIN H17 [get_ports SS_ANODE[1]]; 
set_property PACKAGE_PIN M18 [get_ports SS_ANODE[2]]; 
set_property PACKAGE_PIN L16 [get_ports SS_ANODE[3]]; 
set_property IOSTANDARD LVCMOS33 [get_ports SS_ANODE[*]];

set_property PACKAGE_PIN K14 [get_ports SS_CATHODE[0]]; 
set_property PACKAGE_PIN H15 [get_ports SS_CATHODE[1]]; 
set_property PACKAGE_PIN J18 [get_ports SS_CATHODE[2]]; 
set_property PACKAGE_PIN J15 [get_ports SS_CATHODE[3]]; 
set_property PACKAGE_PIN M17 [get_ports SS_CATHODE[4]]; 
set_property PACKAGE_PIN J16 [get_ports SS_CATHODE[5]]; 
set_property PACKAGE_PIN H18 [get_ports SS_CATHODE[6]]; 
set_property PACKAGE_PIN K18 [get_ports SS_CATHODE[7]]; 
set_property IOSTANDARD LVCMOS33 [get_ports SS_CATHODE[*]];

# Slide switches
set_property PACKAGE_PIN R17 [get_ports SW[0]];
set_property PACKAGE_PIN U20 [get_ports SW[1]];
set_property PACKAGE_PIN R16 [get_ports SW[2]];
set_property PACKAGE_PIN N16 [get_ports SW[3]];
set_property PACKAGE_PIN R14 [get_ports SW[4]];
set_property PACKAGE_PIN P14 [get_ports SW[5]];
set_property PACKAGE_PIN L15 [get_ports SW[6]];
set_property PACKAGE_PIN M15 [get_ports SW[7]];
set_property PACKAGE_PIN T10 [get_ports SW[8]];
set_property PACKAGE_PIN T12 [get_ports SW[9]];
set_property PACKAGE_PIN T11 [get_ports SW[10]];
set_property PACKAGE_PIN T14 [get_ports SW[11]];
set_property IOSTANDARD LVCMOS33 [get_ports SW[*]];

# Push buttons
set_property PACKAGE_PIN W14 [get_ports PB[0]];
set_property PACKAGE_PIN W13 [get_ports PB[1]];
set_property PACKAGE_PIN P15 [get_ports PB[2]];
set_property PACKAGE_PIN M14 [get_ports PB[3]];
set_property IOSTANDARD LVCMOS33 [get_ports PB[*]];

# Pmod header A
set_property PACKAGE_PIN F16 [get_ports GPIO[0]];
set_property PACKAGE_PIN F17 [get_ports GPIO[1]];
set_property PACKAGE_PIN G19 [get_ports GPIO[2]];
set_property PACKAGE_PIN G20 [get_ports GPIO[3]];
set_property PACKAGE_PIN E18 [get_ports GPIO[4]];
set_property PACKAGE_PIN E19 [get_ports GPIO[5]];
set_property PACKAGE_PIN E17 [get_ports GPIO[6]];
set_property PACKAGE_PIN D18 [get_ports GPIO[7]];

# Pmod header B
set_property PACKAGE_PIN D19 [get_ports GPIO[8]];
set_property PACKAGE_PIN D20 [get_ports GPIO[9]];
set_property PACKAGE_PIN F19 [get_ports GPIO[10]];
set_property PACKAGE_PIN F20 [get_ports GPIO[11]];
set_property PACKAGE_PIN C20 [get_ports GPIO[12]];
set_property PACKAGE_PIN B20 [get_ports GPIO[13]];
set_property PACKAGE_PIN B19 [get_ports GPIO[14]];
set_property PACKAGE_PIN A20 [get_ports GPIO[15]];

# Pmod header C
set_property PACKAGE_PIN V15 [get_ports GPIO[16]];
set_property PACKAGE_PIN W15 [get_ports GPIO[17]];
set_property PACKAGE_PIN V16 [get_ports GPIO[18]];
set_property PACKAGE_PIN T16 [get_ports GPIO[19]];
set_property PACKAGE_PIN M19 [get_ports GPIO[20]];
set_property PACKAGE_PIN G14 [get_ports GPIO[21]];
set_property PACKAGE_PIN G17 [get_ports GPIO[22]];
set_property PACKAGE_PIN G15 [get_ports GPIO[23]];
set_property IOSTANDARD LVCMOS33 [get_ports GPIO[*]];

# Servos
set_property PACKAGE_PIN U12 [get_ports SERVO[0]];
set_property PACKAGE_PIN V12 [get_ports SERVO[1]];
set_property PACKAGE_PIN T15 [get_ports SERVO[2]];
set_property PACKAGE_PIN V13 [get_ports SERVO[3]];
set_property IOSTANDARD LVCMOS33 [get_ports SERVO[*]];

# Speaker
set_property PACKAGE_PIN G18 [get_ports PDM_SPEAKER];
set_property IOSTANDARD LVCMOS33 [get_ports PDM_SPEAKER];

# SPK0833LM4H Microphone
set_property PACKAGE_PIN L14 [get_ports PDM_MIC_DATA];
set_property IOSTANDARD LVCMOS33 [get_ports PDM_MIC_DATA];
set_property PACKAGE_PIN N15 [get_ports PDM_MIC_CLK];
set_property IOSTANDARD LVCMOS33 [get_ports PDM_MIC_CLK];

# ESP32 WiFi/Bluetooth
set_property PACKAGE_PIN U14 [get_ports ESP32_UART1_RXD];
set_property IOSTANDARD LVCMOS33 [get_ports ESP32_UART1_RXD];
set_property PACKAGE_PIN U15 [get_ports ESP32_UART1_TXD];
set_property IOSTANDARD LVCMOS33 [get_ports ESP32_UART1_TXD];

# 9LSM9DS 9dof IMU
set_property PACKAGE_PIN H20 [get_ports IMU_SCLK];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_SCLK];
set_property PACKAGE_PIN J19 [get_ports IMU_SDI];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_SDI];
set_property PACKAGE_PIN J20 [get_ports IMU_SDO_AG];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_SDO_AG];
set_property PACKAGE_PIN L17 [get_ports IMU_SDO_M];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_SDO_M];
set_property PACKAGE_PIN K17 [get_ports IMU_CS_AG];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_CS_AG];
set_property PACKAGE_PIN K16 [get_ports IMU_CS_M];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_CS_M];
set_property PACKAGE_PIN L20 [get_ports IMU_DRDY_M];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_DRDY_M];
set_property PACKAGE_PIN M20 [get_ports IMU_INT1_AG];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_INT1_AG];
set_property PACKAGE_PIN L19 [get_ports IMU_INT_M];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_INT_M];
set_property PACKAGE_PIN J14 [get_ports IMU_DEN_AG];
set_property IOSTANDARD LVCMOS33 [get_ports IMU_DEN_AG];

# HDMI data, i2c, and related clocks not added to this version