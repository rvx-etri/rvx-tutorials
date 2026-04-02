# this script works only when 'program_flash' is running on a core

proc check_flash_server {} {
	global OCD_FLASH_CMD_CHECK
	set_gpr 0 $OCD_FLASH_CMD_CHECK
	wait_flash_behavior
}

proc wait_flash_behavior {} {
	global OCD_FLASH_CMD_IDLE
	while 1 {
		set finish [get_gpr 0]
		if {$finish==$OCD_FLASH_CMD_IDLE} {
			break
		} else {
			sleep 10
		}
	}
}

proc erase_flash_sector {addr} {
	global OCD_FLASH_CMD_ERASE

	set_gpr 1 $addr
	set_gpr 0 $OCD_FLASH_CMD_ERASE
	wait_flash_behavior
}

proc write_flash_wo_erase {addr num data_array} {
	global OCD_FLASH_CMD_WRITE
	global OCD_FLASH_CMD_IDLE
	upvar $data_array temp_array

	set_gpr 1 $addr
	set_gpr 2 [expr $num << 2]
	write_memory [get_gpr 3] $num temp_array
	set_gpr 0 $OCD_FLASH_CMD_WRITE
	wait_flash_behavior
}

proc read_flash {addr num data_array} {
	global OCD_FLASH_CMD_READ
	upvar $data_array temp_array

	set_gpr 1 $addr
	set_gpr 2 [expr $num << 2]
	set_gpr 0 $OCD_FLASH_CMD_READ
	wait_flash_behavior
	read_memory [get_gpr 3] $num temp_array
}

proc print_flash_data {addr num} {
	read_flash $addr $num data_array
	for {set i 0} {$i<$num} {set i [expr $i + 1]} {
		set data_array($i) [dec2hex $data_array($i)]
	}
	parray data_array
}

proc program_app_to_flash {addr app_name} {
  global APP_ADDR
	global SMALL_RAM_BASEADDR
	global LARGE_RAM_BASEADDR
	global OCD_FLASH_CMD_WRITE
  global INCLUDE_SDRAM
  global I_SYSTEM_SDRAM_CTRL_BASEADDR
	
	set buffer_size 0x1000
	set sector_size 0x10000
	
	set sram_hex_path ./${app_name}.sram.hex
	if [file exists $sram_hex_path]!=1 {
		echo [format "\[OCD|ERROR\] no hex file \"%s\"" $sram_hex_path]
		break
	}
	set sram_hex_size [get_hex_file_size $sram_hex_path]
	set sram_contents_size [expr $sram_hex_size + 2]

	set dram_hex_path ./${app_name}.dram.hex
	if [info exists LARGE_RAM_BASEADDR]==1 {
		if [file exists $dram_hex_path]!=1 {
			echo [format "\[OCD|ERROR\] no hex file \"%s\"" $dram_hex_path]
			break
		} else {
			set dram_hex_size [get_hex_file_size $dram_hex_path]
		}
	} else {
		set dram_hex_size 0
	}
	if {$dram_hex_size==0} {
		set dram_contents_size 0
	} else {
		set dram_contents_size [expr $dram_hex_size + 2]
	}
	
	set total_contents_size [expr $sram_contents_size + $dram_contents_size + 1]	
	set num_sector [expr [expr [expr [expr $total_contents_size + 1] - 1] / $sector_size] + 1]

	# erase sectors
	check_flash_server
	for {set i 0} {$i<$num_sector} {set i [expr $i + 1]} {
		set sector_addr [expr $addr + [expr $sector_size * $i]]
		erase_flash_sector $sector_addr
	}

	# prepare
	set flash_addr $addr
	set buffered_bytes 0
	set memory_addr [get_gpr 3]

  # write sdram config
  if [info exists INCLUDE_SDRAM]==1 {
    # size
    write_single $memory_addr 12
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    # addr
    write_single $memory_addr $I_SYSTEM_SDRAM_CTRL_BASEADDR
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    # wunit
    write_single $memory_addr 4
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    # data 0
    set config [read_single [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 0]]
    write_single $memory_addr $config
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    # data 4
    set config [read_single [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 4]]
    write_single $memory_addr $config
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    # data 8
    set config [read_single [expr $I_SYSTEM_SDRAM_CTRL_BASEADDR + 8]]
    write_single $memory_addr $config
    set buffered_bytes [expr $buffered_bytes + 4]
    set memory_addr [expr $memory_addr + 4]
    print_sdram_config
  }

  ## write app_addr
  # size
  write_single $memory_addr 4
  set buffered_bytes [expr $buffered_bytes + 4]
  set memory_addr [expr $memory_addr + 4]
  # addr
  write_single $memory_addr [get_app_addr_addr]
  set buffered_bytes [expr $buffered_bytes + 4]
  set memory_addr [expr $memory_addr + 4]
  # wunit
  write_single $memory_addr 4
  set buffered_bytes [expr $buffered_bytes + 4]
  set memory_addr [expr $memory_addr + 4]
  # data
  write_single $memory_addr $APP_ADDR
  set buffered_bytes [expr $buffered_bytes + 4]
  set memory_addr [expr $memory_addr + 4]

	## write sram contents ##
	set hex_path $sram_hex_path
	set hex_size $sram_hex_size
	if {$hex_size==0} {
		set target_addr $SMALL_RAM_BASEADDR
	} else {
		set target_addr $SMALL_RAM_BASEADDR
	}

	if {$hex_size > 0} {
		# read file
		set f [open $hex_path r]
		echo [format "\n\[OCD|INFO\] hex to flash \"%s\" %d bytes" $hex_path $hex_size]

		# write size to memory
		write_single $memory_addr $hex_size
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

		# write addr to memory
		write_single $memory_addr $target_addr
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

    # write wunit to memory
		write_single $memory_addr 1
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

		# write hex to memory
		while {[eof $f]==0} {
			set data {}
			for {set i 0} {$i<8} {set i [expr $i + 2]} {
				set buffer [read $f 2]
				set data [append buffer $data]
			}
			if {[string length $data]!=0} {
				set buffer "0x"
				set data [append buffer $data]
				#echo [format "addr: %x data: %x" $memory_addr $data]
				write_single $memory_addr $data
			}
			set buffered_bytes [expr $buffered_bytes + 4]
			set memory_addr [expr $memory_addr + 4]
			# read \n
			read $f 1

			if {$buffered_bytes==$buffer_size} {
				# program flash
				set_gpr 1 $flash_addr
				set_gpr 2 $buffered_bytes
				set_gpr 0 $OCD_FLASH_CMD_WRITE
				wait_flash_behavior
				set flash_addr [expr $flash_addr + $buffered_bytes]
				set buffered_bytes 0
				set memory_addr [get_gpr 3]
			}
		}
	}

	## write dram contents ##
	set hex_path $dram_hex_path
	set hex_size $dram_hex_size
	if {$hex_size==0} {
		set target_addr $SMALL_RAM_BASEADDR
	} else {
		set target_addr $LARGE_RAM_BASEADDR
	}

	if {$hex_size > 0} {
		# read file
		set f [open $hex_path r]
		echo [format "\n\[OCD|INFO\] hex to flash \"%s\" %d bytes" $hex_path $hex_size]

		# write size to memory
		write_single $memory_addr $hex_size
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

		# write addr to memory
		write_single $memory_addr $target_addr
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

    # write wunit to memory
		write_single $memory_addr 1
		set buffered_bytes [expr $buffered_bytes + 4]
		set memory_addr [expr $memory_addr + 4]

		if {$buffered_bytes==$buffer_size} {
			set_gpr 1 $flash_addr
			set_gpr 2 $buffered_bytes
			set_gpr 0 $OCD_FLASH_CMD_WRITE
			wait_flash_behavior
			set flash_addr [expr $flash_addr + $buffered_bytes]
			set buffered_bytes 0
			set memory_addr [get_gpr 3]
		}

		# write hex to memory
		while {[eof $f]==0} {
			set data {}
			for {set i 0} {$i<8} {set i [expr $i + 2]} {
				set buffer [read $f 2]
				set data [append buffer $data]
			}
			if {[string length $data]!=0} {
				set buffer "0x"
				set data [append buffer $data]
				#echo [format "addr: %x data: %x" $memory_addr $data]
				write_single $memory_addr $data
			}
			set buffered_bytes [expr $buffered_bytes + 4]
			set memory_addr [expr $memory_addr + 4]
			# read \n
			read $f 1

			if {$buffered_bytes==$buffer_size} {
				# program flash
				set_gpr 1 $flash_addr
				set_gpr 2 $buffered_bytes
				set_gpr 0 $OCD_FLASH_CMD_WRITE
				wait_flash_behavior
				set flash_addr [expr $flash_addr + $buffered_bytes]
				set buffered_bytes 0
				set memory_addr [get_gpr 3]
			}
		}
	}

	## write zero to memory ##
	write_single $memory_addr 0
	set buffered_bytes [expr $buffered_bytes + 4]
	set memory_addr [expr $memory_addr + 4]

	## write back the remains ##
	if {$buffered_bytes>0} {
		set_gpr 1 $flash_addr
		set_gpr 2 $buffered_bytes
		set_gpr 0 $OCD_FLASH_CMD_WRITE
		wait_flash_behavior
	}
}

proc test_flash {} {
	set a(0) 4
	set a(1) 1
	set a(2) 7
	set a(3) 9
	set a(4) 9
	set a(5) 3
	set a(6) 2
	set a(7) 8

	set size 8

	set flash_addr [get_flash_base_addr]
	erase_flash_sector $flash_addr
	write_flash_wo_erase $flash_addr $size a
	mww [get_gpr 3] 0 $size
	print_flash_data $flash_addr $size
}

proc test_hex_to_flash {} {
	set flash_addr [get_flash_base_addr]
	program_app_to_flash $flash_addr hello
	print_flash_data $flash_addr 8
}

proc program_flash {app_name} {
	global PLATFORM_NAME
	global SMALL_RAM_BASEADDR
	global BOOT_MODE_OCD
	
	echo [format "\[OCD|INFO\] app \'%s\' will be programmed to flash" $app_name]
	set flash_addr [get_flash_base_addr]
	program_app_to_flash $flash_addr $app_name
	print_flash_data $flash_addr 8

	echo [format "\[OCD|INFO\] programmed successfully"]
}
