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
`include "ervp_cache_memorymap_offset.vh"

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

  core0_rlqdready,
  core0_rlqvalid,
  core0_rlqhint,
  core0_rlqlast,
  core0_rlqafy,
  core0_rlqdata,
  core0_rlydready,
  core0_rlyvalid,
  core0_rlyhint,
  core0_rlylast,
  core0_rlydata,

	cache_sxarid,
	cache_sxaraddr,
	cache_sxarlen,
	cache_sxarsize,
	cache_sxarburst,
	cache_sxarvalid,
	cache_sxarready,

	cache_sxrid,
	cache_sxrdata,
	cache_sxrresp,
	cache_sxrlast,
	cache_sxrvalid,
	cache_sxrready,

	cache_sxawid,
  cache_sxawaddr,
  cache_sxawlen,
  cache_sxawsize,
  cache_sxawburst,
  cache_sxawvalid,
  cache_sxawready,

  cache_sxwid,
  cache_sxwdata,
  cache_sxwstrb,
  cache_sxwlast,
  cache_sxwvalid,
  cache_sxwready,

  cache_sxbid,
  cache_sxbresp,
  cache_sxbvalid,
  cache_sxbready
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 32;
parameter BW_AXI_DATA = 32;
parameter BW_AXI_TID = 4;
parameter CACHE_POLICY = `CACHE_POLICY_WRITE_BACK;
parameter CACHE_SIZE = 4096*2;

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

`include "pact_para_matrix.vb"
`include "pact_para_lsu.vb"

localparam CACHE_LINE_SIZE = 4*(`NUM_BYTE(BW_AXI_DATA));
localparam BW_ACCESS = BW_AXI_DATA;

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
output wire finish;

input wire [`BW_PACT_SUBOP-1:0] subop;
input wire [`BW_PACT_IMMEDIATE-1:0] immediate_value;

`include "lpit_function.vb"
`include "lpimo_function.vb"

localparam BW_LPIMO_ADDR = BW_ADDR;
localparam BW_LPIMO_DATA = BW_ACCESS;
localparam BW_LPI_BURDEN = 0;

`include "lpimo_lpara.vb"

output wire [2-1:0] core0_rlqdready;
input wire core0_rlqvalid;
input wire core0_rlqhint;
input wire core0_rlqlast;
input wire core0_rlqafy;
input wire [BW_LPI_QDATA-1:0] core0_rlqdata;
input wire [2-1:0] core0_rlydready;
output wire core0_rlyvalid;
output wire core0_rlyhint;
output wire core0_rlylast;
output wire [BW_LPI_YDATA-1:0] core0_rlydata;

output wire [BW_AXI_TID-1:0] cache_sxarid;
output wire [BW_ADDR-1:0] cache_sxaraddr;
output wire [`BW_AXI_ALEN-1:0] cache_sxarlen;
output wire [`BW_AXI_ASIZE-1:0] cache_sxarsize;
output wire [`BW_AXI_ABURST-1:0] cache_sxarburst;
output wire cache_sxarvalid;
input wire cache_sxarready;

input wire [BW_AXI_TID-1:0] cache_sxrid;
input wire [BW_AXI_DATA-1:0] cache_sxrdata;
input wire [`BW_AXI_RRESP-1:0] cache_sxrresp;
input wire cache_sxrlast;
input wire cache_sxrvalid;
output wire cache_sxrready;

output wire [BW_AXI_TID-1:0] cache_sxawid;
output wire [BW_ADDR-1:0] cache_sxawaddr;
output wire [`BW_AXI_ALEN-1:0] cache_sxawlen;
output wire [`BW_AXI_ASIZE-1:0] cache_sxawsize;
output wire [`BW_AXI_ABURST-1:0] cache_sxawburst;
output wire cache_sxawvalid;
input wire cache_sxawready;

output wire [BW_AXI_TID-1:0] cache_sxwid;
output wire [BW_AXI_DATA-1:0] cache_sxwdata;
output wire [`BW_AXI_WSTRB(BW_AXI_DATA)-1:0] cache_sxwstrb;
output wire cache_sxwlast;
output wire cache_sxwvalid;
input wire cache_sxwready;

input wire [BW_AXI_TID-1:0] cache_sxbid;
input wire [`BW_AXI_BRESP-1:0] cache_sxbresp;
input wire cache_sxbvalid;
output wire cache_sxbready;

/////////////
/* signals */
/////////////

wire cache_control_valid;
wire cache_control_ready;
wire [`BW_CACHE_CONTROL_CMD-1:0] cache_control_command;
wire [BW_ADDR-1:0] cache_control_base;
wire [BW_ADDR-1:0] cache_control_last;
wire cache_busy;	
wire cache_control_busy;

localparam NUM_EXTERNAL_INTERFACE = 1;
localparam ACCESS_BW_LPI_BURDEN = NUM_EXTERNAL_INTERFACE+1;
localparam ACCESS_BW_LPI_QDATA = (ACCESS_BW_LPI_BURDEN-BW_LPI_BURDEN) + BW_LPI_QDATA;
localparam ACCESS_BW_LPI_YDATA = (ACCESS_BW_LPI_BURDEN-BW_LPI_BURDEN) + BW_LPI_YDATA;

wire [2*NUM_EXTERNAL_INTERFACE-1:0] external_lqdready_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lqvalid_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lqhint_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lqlast_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lqafy_list;
wire [BW_LPI_QDATA*NUM_EXTERNAL_INTERFACE-1:0] external_lqdata_list;
wire [2*NUM_EXTERNAL_INTERFACE-1:0] external_lydready_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lyvalid_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lyhint_list;
wire [NUM_EXTERNAL_INTERFACE-1:0] external_lylast_list;
wire [BW_LPI_YDATA*NUM_EXTERNAL_INTERFACE-1:0] external_lydata_list;

wire [2-1:0] access_lqdready;
wire access_lqvalid;
wire access_lqhint;
wire access_lqlast;
wire access_lqafy;
wire [ACCESS_BW_LPI_QDATA-1:0] access_lqdata;
wire [2-1:0] access_lydready;
wire access_lyvalid;
wire access_lyhint;
wire access_lylast;
wire [ACCESS_BW_LPI_YDATA-1:0] access_lydata;

////////////
/* logics */
////////////

assign external_lqhint_list = core0_rlqhint;
assign external_lqvalid_list = core0_rlqvalid;
assign core0_rlqdready = external_lqdready_list;
assign external_lqafy_list = core0_rlqafy;
assign external_lqdata_list = core0_rlqdata;

assign core0_rlyhint = external_lyhint_list;
assign core0_rlyvalid = external_lyvalid_list;
assign external_lydready_list = core0_rlydready;
assign core0_rlylast = external_lylast_list;
assign core0_rlydata = external_lydata_list;

PACT_LSU
#(
  .BW_ADDR(BW_ADDR),
  .BW_ACCESS(BW_ACCESS),
  .NUM_EXTERNAL_INTERFACE(NUM_EXTERNAL_INTERFACE)
)
i_lsu
(
	.clk(clk),
	.rstnn(rstnn),
  .enable(1'b 1),

	.rreg_sme_list(rreg_sme_list),
	.rreg_smvalue_list(rreg_smvalue_list),
	.wreg_rme_list(wreg_rme_list),
	.wreg_rmvalue_list(wreg_rmvalue_list),

	.start(start),
	.finish(finish),

	.subop(subop),
	.immediate_value(immediate_value),

  .cache_control_valid(cache_control_valid),
  .cache_control_ready(cache_control_ready),
  .cache_control_command(cache_control_command),
  .cache_control_base(cache_control_base),
  .cache_control_last(cache_control_last),

  .cache_busy(cache_busy),
  .cache_control_busy(cache_control_busy),

  .external_rlqdready_list(external_lqdready_list),
  .external_rlqvalid_list(external_lqvalid_list),
  .external_rlqhint_list(external_lqhint_list),
  .external_rlqlast_list(external_lqlast_list),  
  .external_rlqafy_list(external_lqafy_list),
  .external_rlqdata_list(external_lqdata_list),
  .external_rlydready_list(external_lydready_list),
  .external_rlyvalid_list(external_lyvalid_list),
  .external_rlyhint_list(external_lyhint_list),
  .external_rlylast_list(external_lylast_list),
  .external_rlydata_list(external_lydata_list),

  .access_slqdready(access_lqdready),
  .access_slqvalid(access_lqvalid),
  .access_slqhint(access_lqhint),
  .access_slqlast(access_lqlast),
  .access_slqafy(access_lqafy),
  .access_slqdata(access_lqdata),
  .access_slydready(access_lydready),
  .access_slyvalid(access_lyvalid),
  .access_slyhint(access_lyhint),
  .access_slylast(access_lylast),
  .access_slydata(access_lydata)
);

ERVP_CACHE_LS
#(
	.BW_ADDR(BW_ADDR),
	.CACHE_SIZE(CACHE_SIZE),
	.CACHE_LINE_SIZE(CACHE_LINE_SIZE),
	.BW_ACCESS(BW_ACCESS),
	.BW_AXI_DATA(BW_AXI_DATA),
	.BW_AXI_TID(BW_AXI_TID),
	.CACHE_POLICY(CACHE_POLICY),
  .BW_LPI_BURDEN(ACCESS_BW_LPI_BURDEN)
)
i_cache
(
	.clk(clk),
	.rstnn(rstnn),
  .uses_synch_rdata(1'b 0),
  .uses_dependant_ready(1'b 0),
  .clear(1'b 0),
  .enable(1'b 1),
	.busy(cache_busy),
	
	.control_busy(cache_control_busy),
  .control_ready(cache_control_ready),
	.control_valid(cache_control_valid),
	.control_command(cache_control_command),
	.control_base(cache_control_base),
	.control_last(cache_control_last),

	.access_rlqhint(access_lqhint),
  .access_rlqvalid(access_lqvalid),
  .access_rlqready(access_lqdready[0]),
  .access_rlqafy(access_lqafy),
  .access_rlqdata(access_lqdata),
  .access_rlyhint(access_lyhint),
  .access_rlyvalid(access_lyvalid),
  .access_rlydready(access_lydready),
  .access_rlylast(access_lylast),
  .access_rlydata(access_lydata),

	.sxrready(cache_sxrready),
	.sxrvalid(cache_sxrvalid),
	.sxrlast(cache_sxrlast),
	.sxrresp(cache_sxrresp),
	.sxrdata(cache_sxrdata),
	.sxrid(cache_sxrid),
	.sxarready(cache_sxarready),
	.sxarvalid(cache_sxarvalid),
	.sxarburst(cache_sxarburst),
	.sxarsize(cache_sxarsize),
	.sxarlen(cache_sxarlen),
	.sxaraddr(cache_sxaraddr),
	.sxarid(cache_sxarid),
	.sxbready(cache_sxbready),
	.sxbvalid(cache_sxbvalid),
	.sxbresp(cache_sxbresp),
	.sxbid(cache_sxbid),
	.sxwready(cache_sxwready),
	.sxwvalid(cache_sxwvalid),
	.sxwlast(cache_sxwlast),
	.sxwstrb(cache_sxwstrb),
	.sxwdata(cache_sxwdata),
	.sxwid(cache_sxwid),
	.sxawready(cache_sxawready),
	.sxawvalid(cache_sxawvalid),
	.sxawburst(cache_sxawburst),
	.sxawsize(cache_sxawsize),
	.sxawlen(cache_sxawlen),
	.sxawaddr(cache_sxawaddr),
	.sxawid(cache_sxawid)
);

endmodule
