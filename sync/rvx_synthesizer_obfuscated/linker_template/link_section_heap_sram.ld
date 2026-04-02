  .heap_sram :
	{
		. = ALIGN("${DATA_ALIGN_SIZE}");
		_heap_sram_baseaddr_ = .;
	} >sram
