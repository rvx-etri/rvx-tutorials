proc hex2dec {hex} {
	scan $hex %x dec
	return $dec
}

proc dec2hex {dec} {
	set hex [format %X $dec]
	return $hex
}

proc get_hex_file_size {hex_file} {
	
	if [file exists $hex_file]!=1 {
		echo [format "\[ERROR\] no hex file \"%s\"" $hex_file]
		exit 1
	}
	set f [open $hex_file r]
	set num 0
	while {[eof $f]==0} {
		read $f 8
		set num [expr $num + 4]
		read $f 1 
	}
	close $f
	return $num
}
