set ::MASTER_IP 0
set ::SLAVE_IP 1

proc get_noc_mmap {reg} {
	global SUBMODULE_ADDR_ERVP_PLATFORM_CONTROLLER_EXTERNAL
	return [expr $SUBMODULE_ADDR_ERVP_PLATFORM_CONTROLLER_EXTERNAL + $reg]
}

proc get_noc_mmap2 {value reg} {
	global BW_MMAP_OFFSET_MUNOC
	return [expr [get_noc_mmap $reg] + [expr $value << $BW_MMAP_OFFSET_MUNOC]]
}

proc get_elog_mmap {index reg} {
	return [get_noc_mmap2 $index $reg]
}

proc get_ni_mmap {ip_type index reg} {
	global MASTER_IP
	return [get_noc_mmap2 [expr $ip_type + [expr $index << 1]] $reg]
}

proc get_number_of_master {} {
	global MMAP_OFFSET_INFO_NUM_MASTER
	return [read_system [get_noc_mmap $MMAP_OFFSET_INFO_NUM_MASTER]]
}

proc get_number_of_slave {} {
	global MMAP_OFFSET_INFO_NUM_SLAVE
	return [read_system [get_noc_mmap $MMAP_OFFSET_INFO_NUM_SLAVE]]
}

proc print_noc_version {} {
	global MUNOC_ADDR_INTERVAL
	global MMAP_OFFSET_INFO_VERSION0

	set temp_string_size 16
	set temp_string [__read_mmio_string [get_noc_mmap $MMAP_OFFSET_INFO_VERSION0] $temp_string_size $MUNOC_ADDR_INTERVAL]
	puts [format "\[NOC:INFO\] %s" $temp_string]
}

proc get_noc_config {} {
	global MUNOC_ADDR_INTERVAL
	global MMAP_OFFSET_INFO_CONFIG0

	set temp_string_size 16
	set temp_string [__read_mmio_string [get_noc_mmap $MMAP_OFFSET_INFO_CONFIG0] $temp_string_size $MUNOC_ADDR_INTERVAL]
	return $temp_string
}

proc print_noc_config {} {
	puts [format "\[NOC:INFO\] Config: %s" [get_noc_config]]
}

proc print_noc_ip_number {} {
	puts [format "\[NOC:INFO\] master: %d, slave: %d" [get_number_of_master] [get_number_of_slave] ]
}

proc print_noc_info {} {
	print_noc_version
	print_noc_config
	print_noc_ip_number
}

proc get_type_name {type} {
	global MASTER_IP
	global SLAVE_IP
	if {$type==$MASTER_IP} {
		set type_name "master"
	} elseif {$type==$SLAVE_IP} {
		set type_name "slave"
	}
	return $type_name
}

proc print_ip_name {type index} {
	global MUNOC_ADDR_INTERVAL
	global MMAP_OFFSET_CONTROLLER_NAME0
	global MMAP_OFFSET_CONTROLLER_NODEID
	set temp_string_size 32

	set noc_config [get_noc_config]
	if {[string index $noc_config 1]=="C"} {
		set node_id [read_system [get_ni_mmap $type $index $MMAP_OFFSET_CONTROLLER_NODEID]]
		set temp_string [__read_mmio_string [get_ni_mmap $type $index $MMAP_OFFSET_CONTROLLER_NAME0] $temp_string_size $MUNOC_ADDR_INTERVAL]
		puts [format "\[NOC:INFO\] %s %d: %s" [get_type_name $type] $node_id $temp_string]
	}
}

proc print_ip_list {} {
	global MASTER_IP
	global SLAVE_IP
	for {set i 0} {$i<[get_number_of_master]} {set i [expr $i + 1]} {
		print_ip_name $MASTER_IP $i
	}
	for {set i 0} {$i<[get_number_of_slave]} {set i [expr $i + 1]} {
		print_ip_name $SLAVE_IP $i
	}
}

proc check_ip_status {ip_type index} {
	global MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE
	global MMAP_OFFSET_CONTROLLER_EXCLUDE
	global MMAP_OFFSET_CONTROLLER_FNI_STATE
	global MMAP_OFFSET_CONTROLLER_BNI_STATE
	global MMAP_OFFSET_CONTROLLER_IP_TIMEOUT
	global MMAP_OFFSET_CONTROLLER_PROTOCOL_VIOLATION
	global MMAP_OFFSET_CONTROLLER_ROUTING_ERROR
	global MMAP_OFFSET_CONTROLLER_NOC_TIMEOUT

	set noc_config [get_noc_config]
	
	if {[string index $noc_config 1]=="C"} {
		puts [format "Monitor Enabled: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_MONITOR_ENABLE]]]
		puts [format "Exclude: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_EXCLUDE]]]
		puts [format "Send State: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_FNI_STATE]]]
		puts [format "Receive State: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_BNI_STATE]]]
	}

	if {[string index $noc_config 2]=="T"} {
		puts [format "IP Timeout: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_IP_TIMEOUT]]]
		puts [format "NoC Timeout: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_NOC_TIMEOUT]]]
	}
	if {[string index $noc_config 3]=="R"} {
		puts [format "Routing Error: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_ROUTING_ERROR]]]
	}
	if {[string index $noc_config 4]=="V"} {
		puts [format "Protocol Violation: %d" [read_system [get_ni_mmap $ip_type $index $MMAP_OFFSET_CONTROLLER_PROTOCOL_VIOLATION]]]
	}	
	puts "\n"
}

proc check_noc_status {} {
	global MASTER_IP
	global SLAVE_IP

	set noc_config [get_noc_config]
	
	#check_undefined_access();
	if {[string index $noc_config 1]=="C"} {
		for {set i 0} {$i<[get_number_of_master]} {set i [expr $i + 1]} {
			print_ip_name $MASTER_IP $i
			check_ip_status $MASTER_IP $i
		}
		for {set i 0} {$i<[get_number_of_slave]} {set i [expr $i + 1]} {
			print_ip_name $SLAVE_IP $i
			check_ip_status $SLAVE_IP $i
		}
	} else {
		puts "\[NOC:INFO\] Controllers are NOT included"
	}
}
