	.notcached :
	{
		. = ALIGN(8);
		*(.notcached)
		. = ALIGN(8);
	} >"${UNCACHEABLE_MEMORY_REGION}"
