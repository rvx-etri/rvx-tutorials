proc __is_allowed_char {char} {
	switch -exact -- $char {
		" " - "." - "\\" - "_" - ":" - "-" - "/" - "\0" { set result 1}
		default {
			scan $char %c ascii
			if [expr [expr $ascii >= [scan "0" %c]] && [expr $ascii <= [scan "9" %c]]] {
				set result 1
			} elseif [expr [expr $ascii >= [scan "a" %c]] && [expr $ascii <= [scan "z" %c]]] {
				set result 1
			} elseif [expr [expr $ascii >= [scan "A" %c]] && [expr $ascii <= [scan "Z" %c]]] {
				set result 1
			} else {
				set result 0
			}
		}
	}
	return $result
}

proc __read_mmio_string {addr max_size addr_interval} {
	set result ""
	set valid 0
	for {set i 0} {$i < $max_size} {incr i} {
		set i_4 [expr $i % 4]
		if {$i_4==0} {
			set temp [read_system [expr $addr + ($addr_interval/4)*$i]]
		}
		switch -exact -- $i_4 {
			0 { set temp_char_ascii [expr $temp & [hex2dec 0xFF]] }
			1 { set temp_char_ascii [expr [expr $temp >> 8] & [hex2dec 0xFF]] }
			2 { set temp_char_ascii [expr [expr $temp >> 16] & [hex2dec 0xFF]] }
			3 { set temp_char_ascii [expr [expr $temp >> 24] & [hex2dec 0xFF]] }
		}
		set temp_char [format "%c" $temp_char_ascii]
		#echo [format "ascii %d" $temp_char_ascii]
		#echo [format "char %s" $temp_char]

		if {$valid==0} {
			if {$temp_char==" "} {
			} else {
				set valid 1
			}
		}
		if {$valid==1} {
			if {$temp_char=="\0"} {
				break
			} elseif {[__is_allowed_char $temp_char]==0} {
				echo [format "Invalid char: %s 0x%x" $temp_char $temp_char_ascii]
				break
			} else {
				append result $temp_char
			}
		}
	}
	return $result
}
