proc reset_platform {} {
	_reset_platform
	sleep 100
}
proc release_all_reset {} {
	global MMAP_OFFSET_RESET_CMD
	global RESET_CMD_AUTO_INCR
	write_system $MMAP_OFFSET_RESET_CMD $RESET_CMD_AUTO_INCR
	sleep 100
}

proc release_next_reset {} {
	global MMAP_OFFSET_RESET_CMD
	global RESET_CMD_NEXT_STEP
	write_system $MMAP_OFFSET_RESET_CMD $RESET_CMD_NEXT_STEP
	sleep 100
}

proc set_all_ready {} {
	global MMAP_OFFSET_PLATFORM_REGISTER_BOOT_STATUS
	read_system $MMAP_OFFSET_PLATFORM_REGISTER_BOOT_STATUS
	write_system $MMAP_OFFSET_PLATFORM_REGISTER_BOOT_STATUS 1
}

proc set_reset_mask {mask} {
	global MMAP_OFFSET_RESET_MASK

	write_system $MMAP_OFFSET_RESET_MASK $mask
	read_system $MMAP_OFFSET_RESET_MASK
}

proc check_app_status {} {
	global MMAP_OFFSET_PLATFORM_REGISTER_APP_STATUS
	echo [read_system $MMAP_OFFSET_PLATFORM_REGISTER_APP_STATUS]
}

proc print_all_pc {} {
	global MMAP_OFFSET_PLATFORM_REGISTER_CORE_PC00
	global NUM_CORE
	set addr $MMAP_OFFSET_PLATFORM_REGISTER_CORE_PC00
	for {set i 0} {$i<$NUM_CORE} {incr i} {
		echo [format "\[INFO\] core %02d pc: 0x%08x" $i [read_system $addr]]
		set addr [expr $addr + 4]
	}
}

proc print_pc {core_id num} {
	global MMAP_OFFSET_PLATFORM_REGISTER_CORE_PC00
  global BW_MMAP_SUBOFFSET_PLATFORM_REGISTER
	global NUM_CORE
	set addr [expr $MMAP_OFFSET_PLATFORM_REGISTER_CORE_PC00 + [expr $core_id * $BW_MMAP_SUBOFFSET_PLATFORM_REGISTER]]
	for {set i 0} {$i<$num} {incr i} {
		echo [format "\[INFO\] core %02d pc: 0x%08x" $core_id [read_system $addr]]
	}
}

proc reset_all_platform {} {
	reset_platform
	release_all_reset
}
