proc read_system {addr} {
	_read_system $addr data
	return $data
}

proc read_memory {addr} {
	set data_array(0) 0x0
	mem2array data_array 32 $addr 1
	return $data_array(0)
}

proc write_memory {addr data} {
	set data_array(0) $data
	array2mem data_array 32 $addr 1
}
