#synthesis
reset_run synth_1
launch_runs synth_1 -jobs 16
wait_on_run synth_1

#implementation
reset_run impl_1
launch_runs impl_1 -jobs 16 -to_step write_bitstream
wait_on_run impl_1

#program device
open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target
set_property PROBES.FILE {} [get_hw_devices xc7z007s_1]
set_property FULL_PROBES.FILE {} [get_hw_devices xc7z007s_1]
set_property PROGRAM.FILE {/home/jlosh/Documents/SoC/Zynq/FPGA/combo_logic/combo_logic.runs/impl_1/combo_logic.bit} [get_hw_devices xc7z007s_1]
program_hw_devices [get_hw_devices xc7z007s_1]
refresh_hw_device [lindex [get_hw_devices xc7z007s_1] 0]
close_hw_manager

puts "Steps complete"

