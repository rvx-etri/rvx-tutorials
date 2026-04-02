////////////
/* logics */
////////////

assign man_rpwdata = CHANGE_ENDIAN_BUS2MAN(BW_DATA,ENDIAN_TYPE,rpwdata);
assign rprdata = CHANGE_ENDIAN_MAN2BUS(BW_DATA,ENDIAN_TYPE,man_rprdata);
assign {addr_aligned,addr_unaligned} = paddr_offset;
assign addr_offset = {addr_aligned,addr_unused};
assign is_aligned_access = (addr_unaligned==0);
assign read_request_from_bus = rpsel & rpenable & is_aligned_access & (~rpwrite);
assign write_request_from_bus = rpsel & rpenable & is_aligned_access & rpwrite;
