	.data :
	{
		. = ALIGN(8);
		*(.data .data.*)
    *(.gnu.linkonce.d.*)
		. = ALIGN(8);
	} >"${MULTI_WRITE_MEMORY_REGION}"

	.sdata :
	{
		. = ALIGN(8);
		_gp = . + 0x800;
		*(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata .srodata.*)
		. = ALIGN(8);
  } >"${MULTI_WRITE_MEMORY_REGION}"

  .sbss :
	{
		. = ALIGN(8);
		PROVIDE( _fbss = . );
		_bss_start_ = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
		. = ALIGN(8);
	} >"${MULTI_WRITE_MEMORY_REGION}"

	.bss :
	{
		. = ALIGN(8);
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		_bss_end_ = NEXT(4);
	} >"${MULTI_WRITE_MEMORY_REGION}"
