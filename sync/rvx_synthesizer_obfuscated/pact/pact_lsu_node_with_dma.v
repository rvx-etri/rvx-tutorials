// ****************************************************************************
// ****************************************************************************
// Copyright SoC Design Research Group, All rights reserved.    
// Electronics and Telecommunications Research Institute (ETRI)
//
// THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
// WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
// TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
// REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
// SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
// IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
// COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
// ****************************************************************************
// 2022-03-24
// Kyuseung Han (han@etri.re.kr)
// ****************************************************************************
// ****************************************************************************

`include "ervp_global.vh"
`include "ervp_axi_define.vh"

`include "pact_extended_config.vh"
`include "pact_memorymap_offset.vh"
`include "pact_memory_config.vh"

module PACT_LSU_NODE
(
	clk,
	rstnn,

	rreg_sme_list,
	rreg_smvalue_list,
	wreg_rme_list,
	wreg_rmvalue_list,

	start,
	finish,

	subop,
	immediate_value,

	dma_sxarid,
	dma_sxaraddr,
	dma_sxarlen,
	dma_sxarsize,
	dma_sxarburst,
	dma_sxarvalid,
	dma_sxarready,

	dma_sxrid,
	dma_sxrdata,
	dma_sxrresp,
	dma_sxrlast,
	dma_sxrvalid,
	dma_sxrready,

	dma_sxawid,
  dma_sxawaddr,
  dma_sxawlen,
  dma_sxawsize,
  dma_sxawburst,
  dma_sxawvalid,
  dma_sxawready,

  dma_sxwid,
  dma_sxwdata,
  dma_sxwstrb,
  dma_sxwlast,
  dma_sxwvalid,
  dma_sxwready,

  dma_sxbid,
  dma_sxbresp,
  dma_sxbvalid,
  dma_sxbready
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 32;
parameter BW_AXI_DATA = 32;
parameter BW_AXI_TID = 4;
parameter CACHE_SIZE = 0;

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

`include "pact_para_matrix.vb"
`include "pact_para_lsu.vb"

localparam NUM_BIPUT_REG = 0;
localparam NUM_INPUT_REG = 1;
localparam NUM_OUTPUT_REG = 1;

localparam NUM_READ_REG = 1;
localparam NUM_WRITE_REG = 1;

input wire clk, rstnn;

input wire [NUM_READ_REG-1:0] rreg_sme_list;
output wire [NUM_READ_REG*BW_MOVE_DATA-1:0] rreg_smvalue_list;
input wire [NUM_WRITE_REG-1:0] wreg_rme_list;
input wire [NUM_WRITE_REG*BW_MOVE_DATA-1:0] wreg_rmvalue_list;

input wire start;
output reg finish;

input wire [`BW_PACT_SUBOP-1:0] subop;
input wire [`BW_PACT_IMMEDIATE-1:0] immediate_value;

output wire [BW_AXI_TID-1:0] dma_sxarid;
output wire [BW_ADDR-1:0] dma_sxaraddr;
output wire [`BW_AXI_ALEN-1:0] dma_sxarlen;
output wire [`BW_AXI_ASIZE-1:0] dma_sxarsize;
output wire [`BW_AXI_ABURST-1:0] dma_sxarburst;
output wire dma_sxarvalid;
input wire dma_sxarready;

input wire [BW_AXI_TID-1:0] dma_sxrid;
input wire [BW_AXI_DATA-1:0] dma_sxrdata;
input wire [`BW_AXI_RRESP-1:0] dma_sxrresp;
input wire dma_sxrlast;
input wire dma_sxrvalid;
output wire dma_sxrready;

output wire [BW_AXI_TID-1:0] dma_sxawid;
output wire [BW_ADDR-1:0] dma_sxawaddr;
output wire [`BW_AXI_ALEN-1:0] dma_sxawlen;
output wire [`BW_AXI_ASIZE-1:0] dma_sxawsize;
output wire [`BW_AXI_ABURST-1:0] dma_sxawburst;
output wire dma_sxawvalid;
input wire dma_sxawready;

output wire [BW_AXI_TID-1:0] dma_sxwid;
output wire [BW_AXI_DATA-1:0] dma_sxwdata;
output wire [`BW_AXI_WSTRB(BW_AXI_DATA)-1:0] dma_sxwstrb;
output wire dma_sxwlast;
output wire dma_sxwvalid;
input wire dma_sxwready;

input wire [BW_AXI_TID-1:0] dma_sxbid;
input wire [`BW_AXI_BRESP-1:0] dma_sxbresp;
input wire dma_sxbvalid;
output wire dma_sxbready;

/////////////
/* signals */
/////////////

wire [`BW_MEMORY_CONFIG_START_ADDR-1:0] start_addr;
wire [`BW_MEMORY_CONFIG_STRIDE-1:0] stride;
wire [`BW_MEMORY_CONFIG_NUM_ROW_M1-1:0] num_row_m1;
wire [`BW_MEMORY_CONFIG_NUM_COL_M1-1:0] num_col_m1;
wire [`BW_MEMORY_CONFIG_DATA_INFO_DATA_WIDTH-1:0] data_width;
wire [`BW_MEMORY_CONFIG_DATA_INFO_IS_FLOAT-1:0] is_float;
wire [`BW_MEMORY_CONFIG_DATA_INFO_IS_SIGNED-1:0] is_signed;

localparam BW_STATE = `BW_PACT_SUBOP_LSU;
localparam IDLE = `PACT_SUBOP_LSU_IDLE;

reg [BW_STATE-1:0] state;
wire load_enable;
wire store_enable;
reg already_started;
reg [`BW_PACT_IMMEDIATE-1:0] immediate_value_stored;
wire is_row_index_invalid;
reg [MATRIX_SIZE-1:0] valid_col_list;

localparam BW_COUNTER = REQUIRED_BITWIDTH_INDEX(MATRIX_SIZE);

wire i_row_counter_enable;
wire i_row_counter_init;
wire i_row_counter_count;
wire i_row_counter_is_last_count;
wire [BW_COUNTER-1:0] i_row_counter_value;

wire reader_start;
wire writer_start;

wire write_memory_config;
wire [`BW_MEMORY_CONFIG-1:0] memory_config;

wire read_txn_info_valid;
wire [BW_TXN_INFO-1:0] read_txn_info_wdata;

wire write_txn_info_valid;
wire [BW_TXN_INFO-1:0] write_txn_info_wdata;
wire write_addr_is_transferred;

wire memory_read_row_valid;
wire [BW_PACT_ROW-1:0] memory_read_row_data;

reg load_row_valid;
wire [BW_PACT_ROW-1:0] load_row_data;

wire loading_row_will_be_complete;
reg shifting_sreg_is_complete;
wire storing_row_will_be_complete;
wire writing_memory_is_busy;

wire lreg_shift_up;
wire lreg_down_wenable;
wire [BW_PACT_ROW-1:0] lreg_down_wdata_list;

reg sreg_shift_up;
wire sreg_shift_up_by_wdata_handler;
wire [BW_PACT_ROW-1:0] sreg_up_data_list;

///////////////
/* registers */
///////////////

integer i;

wire [BW_MOVE_DATA-1:0] move_wdata_list_zero;
wire [BW_PACT_MATRIX-1:0] all_wdata_list2_zero;
wire [BW_PACT_ROW-1:0] right_wdata_list_zero;

PACT_MATRIX_REGISTER_TYPE1
#(
  .BW_DATA(`PACT_BW_SINGLE),
  .MATRIX_SIZE(MATRIX_SIZE),
	.BW_MOVE_DATA(BW_MOVE_DATA)
)
i_load_reg
(
  .clk(clk),
  .rstnn(rstnn),

  .init(1'b 0),

	.move_wenable(1'b 0),
  .move_wdata_list(move_wdata_list_zero),
	.move_renable(rreg_sme_list),
	.move_rdata_list(rreg_smvalue_list),

	.all_wenable(1'b 0),
	.all_wdata_list2(all_wdata_list2_zero),

  .down_wdata_list(lreg_down_wdata_list),
  .down_wenable(lreg_down_wenable),

  .right_wdata_list(right_wdata_list_zero),
  .right_wenable(1'b 0),

	.shift_up(lreg_shift_up),
  .shift_left(1'b 0),
	.transpose(1'b 0),

  .all_data_list2(),
  .up_data_list()
);

assign move_wdata_list_zero = 0;
assign all_wdata_list2_zero = 0;
assign right_wdata_list_zero = 0;

PACT_MATRIX_REGISTER_TYPE3
#(
  .BW_DATA(`PACT_BW_SINGLE),
  .MATRIX_SIZE(MATRIX_SIZE),
	.BW_MOVE_DATA(BW_MOVE_DATA)
)
i_store_reg
(
  .clk(clk),
  .rstnn(rstnn),

  .move_wenable(wreg_rme_list),
  .move_wdata_list(wreg_rmvalue_list),
	.move_renable(1'b 0),
	.move_rdata_list(),

  .shift_up(sreg_shift_up),
  .shift_left(1'b 0),
	.transpose(1'b 0),

  .all_data_list2(),
  .up_data_list(sreg_up_data_list)
);

////////////
/* logics */
////////////

assign {is_signed, is_float, data_width, num_col_m1, num_row_m1, stride, start_addr} = memory_config;

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
	begin
    state <= IDLE;
		immediate_value_stored <= 0;
	end
  else if(state==IDLE)
	begin
		if(start)
		begin
			state <= subop;
			immediate_value_stored <= immediate_value;
		end
	end
	else
	begin
		if(finish)
			state <= IDLE;
	end
end

assign load_enable = (state==`PACT_SUBOP_LSU_LOAD);
assign store_enable = (state==`PACT_SUBOP_LSU_STORE);

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
		already_started <= 0;
	else if(finish==1)
		already_started <= 0;
	else if(reader_start||writer_start)
		already_started <= 1;
end

ERVP_COUNTER
#(
	.BW_COUNTER(BW_COUNTER),
	.LAST_NUMBER(MATRIX_SIZE-1),
	.COUNT_AMOUNT(1)
)
i_row_counter
(
	.clk(clk),
	.rstnn(rstnn),
	.enable(i_row_counter_enable),
	.init(i_row_counter_init),
	.count(i_row_counter_count),
	.value(i_row_counter_value),
	.is_first_count(),
	.is_last_count(i_row_counter_is_last_count)
);

assign i_row_counter_enable = load_enable | store_enable;
assign i_row_counter_init = finish;
assign i_row_counter_count = loading_row_will_be_complete | sreg_shift_up;

assign is_row_index_invalid = ($unsigned(i_row_counter_value)>$unsigned(num_row_m1));

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
		shifting_sreg_is_complete <= 0;
	else if(finish==1)
		shifting_sreg_is_complete <= 0;
	else if(sreg_shift_up&&i_row_counter_is_last_count)
		shifting_sreg_is_complete <= 1;
end

assign storing_row_will_be_complete = shifting_sreg_is_complete & (~writing_memory_is_busy);

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
		valid_col_list <= 0;
	else if(reader_start||writer_start)
	begin
		for(i=0; i<MATRIX_SIZE; i=i+1)
			valid_col_list[i] <= (i<=num_col_m1);
	end
end

assign loading_row_will_be_complete = load_row_valid;

always@(*)
begin
	sreg_shift_up = 0;
	case(state)
		`PACT_SUBOP_LSU_STORE:
			if(is_row_index_invalid)
				sreg_shift_up = ~shifting_sreg_is_complete;
			else
				sreg_shift_up = sreg_shift_up_by_wdata_handler;
	endcase
end

assign lreg_shift_up = loading_row_will_be_complete;
assign lreg_down_wenable = loading_row_will_be_complete;
assign lreg_down_wdata_list = load_row_data;

always@(*)
begin
	finish = 0;
	case(state)
		`PACT_SUBOP_LSU_INFO,
		`PACT_SUBOP_LSU_CLEAN,
		`PACT_SUBOP_LSU_INVALIDATE,
		`PACT_SUBOP_LSU_FLUSH,
		`PACT_SUBOP_LSU_WAITCACHE:
      finish = 1;
		`PACT_SUBOP_LSU_LOAD:
			finish = loading_row_will_be_complete & i_row_counter_is_last_count;
		`PACT_SUBOP_LSU_STORE:
			finish = storing_row_will_be_complete & i_row_counter_is_last_count;
	endcase
end

PACT_MEMORY_CONFIG
#(
	.BW_DATA(`BW_PACT_IMMEDIATE)
)
i_memory_config
(
	.clk(clk),
	.rstnn(rstnn),

	.write(write_memory_config),
	.wdata(immediate_value_stored),
	.memory_config(memory_config)
);

assign write_memory_config = (state==`PACT_SUBOP_LSU_INFO);

PACT_ADDR_HANDLER
#(
	.BW_ADDR(BW_ADDR),
	.BW_AXI_DATA(BW_AXI_DATA),
	.BW_AXI_TID(BW_AXI_TID),
	.MATRIX_SIZE(MATRIX_SIZE)
)
i_axiar_handler
(
  .clk(clk),
  .rstnn(rstnn),
	.enable(load_enable),

  .start(reader_start),
  .memory_config(memory_config),
	.allows_to_optimize(1'b 1),

	.txn_info_valid(read_txn_info_valid),
	.txn_info_wdata(read_txn_info_wdata),
  .addr_is_transferred(),

  .sxaaid(dma_sxarid),
  .sxaaaddr(dma_sxaraddr),
  .sxaavalid(dma_sxarvalid),
  .sxaaready(dma_sxarready),
  .sxaalen(dma_sxarlen),
  .sxaaburst(dma_sxarburst),
  .sxaasize(dma_sxarsize)
);

assign reader_start = (state==`PACT_SUBOP_LSU_LOAD) & (~already_started);

PACT_RDATA_HANDLER
#(
  .BW_AXI_DATA(BW_AXI_DATA)
)
i_rdata_handler
(
  .clk(clk),
  .rstnn(rstnn),
  .enable(load_enable),

	.start(reader_start),
	.memory_config(memory_config),
	.valid_col_list(valid_col_list),

	.txn_info_valid(read_txn_info_valid),
	.txn_info_wdata(read_txn_info_wdata),

  .rvalid(dma_sxrvalid),
  .rready(dma_sxrready),
	.rdata(dma_sxrdata),
  .rlast(dma_sxrlast),

	.memory_read_row_valid(memory_read_row_valid),
  .memory_read_row_data(memory_read_row_data)
);

always@(*)
begin
	load_row_valid = 0;
	case(state)
		`PACT_SUBOP_LSU_LOAD:
			if(is_row_index_invalid)
				load_row_valid = 1;
			else
				load_row_valid = memory_read_row_valid;
	endcase
end

PACT_LOAD_ROW_INVALIDATOR
#(
	.MATRIX_SIZE(MATRIX_SIZE)
)
i_row_invalidator
(
	.clk(clk),
	.rstnn(rstnn),
	.enable(load_enable),

	.is_float(is_float),
	.is_row_index_invalid(is_row_index_invalid),
	.valid_col_list(valid_col_list),

	.input_row_valid(load_row_valid),
	.memory_read_row_data(memory_read_row_data),
	.load_row_data(load_row_data)
);

PACT_ADDR_HANDLER
#(
	.BW_ADDR(BW_ADDR),
	.BW_AXI_DATA(BW_AXI_DATA),
	.BW_AXI_TID(BW_AXI_TID),
	.MATRIX_SIZE(MATRIX_SIZE)
)
i_axiaw_handler
(
  .clk(clk),
  .rstnn(rstnn),
	.enable(store_enable),

  .start(writer_start),
  .memory_config(memory_config),
	.allows_to_optimize(1'b 1),

	.txn_info_valid(write_txn_info_valid),
	.txn_info_wdata(write_txn_info_wdata),
  .addr_is_transferred(write_addr_is_transferred),

  .sxaaid(dma_sxawid),
  .sxaaaddr(dma_sxawaddr),
  .sxaavalid(dma_sxawvalid),
  .sxaaready(dma_sxawready),
  .sxaalen(dma_sxawlen),
  .sxaaburst(dma_sxawburst),
  .sxaasize(dma_sxawsize)
);

PACT_WDATA_HANDLER
#(
  .BW_AXI_DATA(BW_AXI_DATA)
)
i_wdata_handler
(
  .clk(clk),
  .rstnn(rstnn),
  .enable(store_enable),
	.busy(writing_memory_is_busy),

	.start(writer_start),
	.memory_config(memory_config),
	.valid_col_list(valid_col_list),

	.txn_info_valid(write_txn_info_valid),
	.txn_info_wdata(write_txn_info_wdata),
  .addr_is_transferred(write_addr_is_transferred),

	.sreg_shift_up(sreg_shift_up_by_wdata_handler),
	.sreg_up_data_list(sreg_up_data_list),

  .wvalid(dma_sxwvalid),
  .wready(dma_sxwready),
	.wdata(dma_sxwdata),
  .wstrb(dma_sxwstrb),
  .wlast(dma_sxwlast)  
);

assign writer_start = (state==`PACT_SUBOP_LSU_STORE) & (~already_started);

assign dma_sxwid = 0;
assign dma_sxbready = 1;

endmodule
