proc read_memory {addr num var_name} {
	if {$num!=0} {
		upvar $var_name temp_array
		mem2array temp_array 32 $addr $num
	}
}

proc print_memory {addr num} {
	if {$num!=0} {
		read_memory $addr $num var_name
		for {set i 0} {$i<$num} {incr i} {
			echo [format "\[INFO\] addr: 0x%08x value: 0x%08x" $addr $var_name($i)]
			set addr [expr $addr + 4]
		}
	}
}

proc write_memory {addr num var_name} {
	if {$num!=0} {
		upvar $var_name temp_array
		array2mem temp_array 32 $addr $num
	}
}

proc read_single {addr} {
	read_memory $addr 1 temp
	return $temp(0)
}

proc write_single {addr data} {
	set temp(0) $data
	write_memory $addr 1 temp
}

proc write_hex_to_memory {addr hex_file_name ram_size} {
	
	if [file exists $hex_file_name]!=1 {
		echo [format "\n\[MUNOC|ERROR\] no hex file \"%s\"" $hex_file_name]
		break
	} else {
		echo [format "\n\[MUNOC|INFO\] hex file: \"%s\"" $hex_file_name]
		echo [format "\[MUNOC|INFO\] hex size: %d" [expr [file size $hex_file_name] * 4 / 9]]
	}

	set f [open $hex_file_name r]

	set total_size 0
	set max_array_size 1000
	if {[eof $f]==0} {
		while 1 {
			set index 0
			while {$index<$max_array_size} {
				# read single line
				set data {}
				for {set i 0} {$i<8} {set i [expr $i + 2]} {
					set buffer [read $f 2]
					set data [append buffer $data]
				}
				# read \n
				read $f 1
				# store array
				if {[string length $data]!=0} {
					set buffer "0x"
					set data [append buffer $data]
					set hex_data($index) $data
					set index [expr $index + 1]
				}
				# by hujang
				# "[eof $f]" is 16 at end of file in Windows
				if {[eof $f]!=0} {
					break
				}
			}
			#
			write_memory $addr $index hex_data
			set addr [expr $addr + [expr $index << 2]]			
			set total_size [expr $total_size + $index]
			set reminder [expr $total_size % 100000]
			if {$reminder==0} {
				echo [format "writing... %d" $total_size]
			}
			# by hujang
			# "[eof $f]" is 16 at end of file in Windows
			if {[eof $f]!=0} {
				break
			}
		}
	}
	
	# echo [array size hex_data]
	# parray hex_data
	set num_bytes [expr $total_size * 4]
	echo [format "\[MUNOC|INFO\] write hex to memory\n\"%s\"\n%d bytes" $hex_file_name $num_bytes]
	if {$total_size > $ram_size} {
		echo [format "\[ERROR\] the size of app is %d (0x%08x) bytes, which is larger than 0x%x" $num_bytes $num_bytes $ram_size]
	}

	# check
	#mem2array read_data 32 $addr $total_size
	#for {set i 0} {$i<$total_size} {set i [expr $i + 1]} {
	#	if {$hex_data($i)!=$read_data($i)} {
	#		echo [format "\n\[MUNOC|ERROR\] wrong memory write\n 0x%x 0x%08x 0x%08x" [expr $addr + [expr $i * 4]] $hex_data($i) $read_data($i)]
	#	}
	#}
}

proc print_irom {} {
	global EXTERNAL_PERI_GROUP_BASEADDR
	global SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_IROM
	set addr [expr $EXTERNAL_PERI_GROUP_BASEADDR + $SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_IROM]
	print_memory $addr 8
}

proc print_sram {} {
	global SMALL_RAM_BASEADDR
	set addr $SMALL_RAM_BASEADDR
	print_memory $addr 8
}
