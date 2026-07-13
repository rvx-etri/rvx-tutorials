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

proc print_sdram_config {} {
  global I_SYSTEM_SDRAM_CTRL_BASEADDR
  print_memory $I_SYSTEM_SDRAM_CTRL_BASEADDR 3
}

proc init_sdram {} {
  global I_SDRAM_BASEADDR
  global I_SYSTEM_SDRAM_CTRL_BASEADDR
  global SDRAM_CLK_HZ

  set SDR_OFF_CONFIG_REG0 0
  set SDR_OFF_CONFIG_REG1 4
  set SDR_OFF_REF_REG     8

  set configure_is_required 0

  if {[info exists ::LARGE_RAM_BASEADDR]} {
    global LARGE_RAM_BASEADDR
    if {$LARGE_RAM_BASEADDR == $I_SDRAM_BASEADDR} {
      set configure_is_required 1
    }
  }
  if {$configure_is_required == 1} {

    # Automatically issue NOP to the SDRAM
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 0x03
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 1
    after 2

    # Enable SDRAM MODE Command
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 0x01
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_REF_REG}] 0x14
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 1
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_REF_REG}] 1
    after 1

    # Refresh counter setup: 7.8 us
    # 50 MHz * 7.8 us = 390
    set refresh_count [expr {int(7.8 * $SDRAM_CLK_HZ / 1000000.0)}]
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_REF_REG}] $refresh_count
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_REF_REG}] 1

    # Automatically issue PALL to the SDRAM
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 0x02
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 1

    # Read from SDRAM address to set burst length, burst type, CAS latency
    read_single [expr {$I_SDRAM_BASEADDR + 0x32000}]

    # Check status bit[5], wait until idle
    set config_reg1_addr [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}]

    while {1} {
      set sdr_setup [read_single $config_reg1_addr]
      set sdr_setup [expr {$sdr_setup & 0x20}]

      if {$sdr_setup == 0} {
        break
      }
    }

    # Set SDRAM_CONFIG_REG_0
    # FPGA: CAS=3, 2-bank
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG0}] 0x01f40002
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG0}] 1

    # Set normal operation
    write_single [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 0x00
    print_memory [expr {$I_SYSTEM_SDRAM_CTRL_BASEADDR + $SDR_OFF_CONFIG_REG1}] 1

    echo "SDRAM is initialized"
    after 1
  }
}
