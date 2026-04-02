	.init	:
  {
    _text_start_ = .;
		_cacheable_start_ = .;
		*(.text.init)
		. = ALIGN(8);
		*(.init)
		. = ALIGN(8);
  } >"${TEXT_MEMORY_REGION}"

	.text :
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
		. = ALIGN(8);
  } >"${TEXT_MEMORY_REGION}"

  .fini           :
	{
		KEEP (*(SORT_NONE(.fini)))
	} >"${TEXT_MEMORY_REGION}"

	.preinit_array  :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} >"${TEXT_MEMORY_REGION}"

	.init_array     :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	} >"${TEXT_MEMORY_REGION}"

	.fini_array     :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} >"${TEXT_MEMORY_REGION}"

	.ctors          :
	{
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	} >"${TEXT_MEMORY_REGION}"

	.dtors          :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
    _text_end_ = .;
	} >"${TEXT_MEMORY_REGION}"

  .rodata	:
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
    *(.non_volatile)
		. = ALIGN(8);
  } >"${SINGLE_WRITE_MEMORY_REGION}"
	
	.stack :
	{
		. = ALIGN("${DATA_ALIGN_SIZE}");
		. += ("${STACK_SIZE}"*"${NUM_CORE_PHYSICAL}");
		. = ALIGN("${DATA_ALIGN_SIZE}");
		PROVIDE( _sp = . );
	} >"${SINGLE_WRITE_MEMORY_REGION}"

  .bigdata_bss :
	{
		. = ALIGN("${DATA_ALIGN_SIZE}");
		_bigdata_bss_start_ = .;
		*(.bigdata_bss)
		_bigdata_bss_end_ = .;
		_cacheable_end_ = NEXT("${DATA_ALIGN_SIZE}");
	} >"${SINGLE_WRITE_MEMORY_REGION}"

	.bigdata :
	{
    _backup_start_ = .;
		. = ALIGN("${DATA_ALIGN_SIZE}");
		*(.bigdata)
	} >"${SINGLE_WRITE_MEMORY_REGION}"

	.once :
	{
		. = ALIGN("${DATA_ALIGN_SIZE}");
		*(.once)
	} >"${SINGLE_WRITE_MEMORY_REGION}"

  .cached :
	{
		. = ALIGN("${DATA_ALIGN_SIZE}");
		*(.cached)
		*(.exclusive)
  } >"${SINGLE_WRITE_MEMORY_REGION}"
