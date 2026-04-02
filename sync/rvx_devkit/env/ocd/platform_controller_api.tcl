proc get_boot_mode {} {
	global MMAP_OFFSET_PLATFORM_REGISTER_BOOT_MODE
	set addr $MMAP_OFFSET_PLATFORM_REGISTER_BOOT_MODE
	return [read_system $addr]
}

proc get_platform_name {} {
	global ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL
	global MMAP_OFFSET_DESIGN_INFO_PLATFORM_NAME00

	set string_size 16
	set addr $MMAP_OFFSET_DESIGN_INFO_PLATFORM_NAME00
	set result [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]
	return $result
}

proc get_username {} {
	global ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL
	global MMAP_OFFSET_DESIGN_INFO_USERNAME00

	set string_size 16
	set addr $MMAP_OFFSET_DESIGN_INFO_USERNAME00
	set result [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]
	return $result
}

proc get_git_name {} {
	global ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL
	global MMAP_OFFSET_DESIGN_INFO_HOME_GIT_NAME00

	set string_size 16
	set addr $MMAP_OFFSET_DESIGN_INFO_HOME_GIT_NAME00
	set result [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]
	return $result
}

proc get_design_date {} {
	global ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL
	global MMAP_OFFSET_DESIGN_INFO_DATE00

	set string_size 24
	set addr $MMAP_OFFSET_DESIGN_INFO_DATE00
	set result [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]
	return $result
}

proc print_git_version {} {
	global ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL
	global MMAP_OFFSET_DESIGN_INFO_HOME_GIT_VERSION00
	global MMAP_OFFSET_DESIGN_INFO_DEVKIT_GIT_VERSION00

	if {[has_git_info]==1} {
		set string_size 16
		set addr $MMAP_OFFSET_DESIGN_INFO_HOME_GIT_VERSION00
		echo [format "\[OCD|INFO\] hoem git: %s" [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]]
		set addr $MMAP_OFFSET_DESIGN_INFO_DEVKIT_GIT_VERSION00
		echo [format "\[OCD|INFO\] devkit git: %s" [__read_mmio_string $addr $string_size $ERVP_PLATFORM_CONTROLLER_ADDR_INTERVAL]]
	}
}

proc print_platform_info {} {
	set platform_name [get_platform_name]
	set design_date [get_design_date]
	echo [format "\n\[OCD|INFO\] %s @ %s" $platform_name $design_date]
  set username [get_username]
	set git_name [get_git_name]
	echo [format "\n\[OCD|INFO\] %s @ %s" $username $git_name]
}

proc get_flash_base_addr {} {
	global MMAP_OFFSET_PLATFORM_REGISTER_FLASH_BASE_ADDR

	set addr $MMAP_OFFSET_PLATFORM_REGISTER_FLASH_BASE_ADDR
	return [read_system $addr]
}
