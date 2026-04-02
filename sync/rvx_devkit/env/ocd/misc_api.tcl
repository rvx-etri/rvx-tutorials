proc get_gpr_addr {index} {
	global EXTERNAL_PERI_GROUP_BASEADDR
	global MMAP_OFFSET_EPG_MISC_EXTREG00
	global BW_UNUSED_EPG_MISC

	set addr [expr [expr $EXTERNAL_PERI_GROUP_BASEADDR + $MMAP_OFFSET_EPG_MISC_EXTREG00] + [expr $index << $BW_UNUSED_EPG_MISC]]
	return $addr
}

proc get_gpr {index} {
	return [read_single [get_gpr_addr $index]]
}

proc set_gpr {index data} {
	write_single [get_gpr_addr $index] $data
}

proc get_app_addr_addr {} {
	global PLATFORM_CONTROLLER_BASEADDR
	global MMAP_OFFSET_PLATFORM_REGISTER_APP_ADDR

	set addr [expr $PLATFORM_CONTROLLER_BASEADDR + $MMAP_OFFSET_PLATFORM_REGISTER_APP_ADDR]
	return $addr
}

proc get_app_addr {} {
	return [read_single [get_app_addr_addr]]
}

proc set_app_addr {app_addr} {
	write_single [get_app_addr_addr] $app_addr
}

proc init_sdram {clk_pol clk_skew cell_config refresh_cycle delay_config } {
  global I_SYSTEM_SDRAM_CTRL_BASEADDR
  set config $cell_config
  set config [expr $config + [expr $clk_skew << 5]]
  set config [expr $config + [expr $clk_pol << 11]]
  set addr [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 0]
  write_single $addr $config
  set addr [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 4]
  write_single $addr $refresh_cycle
  set addr [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 8]
  write_single $addr $delay_config
  echo "SDRAM is initialized"
}

proc print_sdram_config {} {
  global I_SYSTEM_SDRAM_CTRL_BASEADDR
  print_memory $I_SYSTEM_SDRAM_CTRL_BASEADDR 3
}

