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
// 2022-09-06
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

wire external_bqhint;
wire external_bqvalid;
wire external_bqready;
wire external_bqwrite;
wire [BW_ADDR-1:0] external_bqaddr;
wire [BW_ACCESS-1:0] external_bqwvalue;
wire [`BW_AXI_WSTRB(BW_AXI_DATA)-1:0] external_bqwstrb;
wire external_bpvalid;
wire external_bpready;
wire [BW_AXI_DATA-1:0] external_bprvalue;

////////////
/* logics */
////////////

PACT_LOCAL_LSU_WITH_SPM
#(
  .BW_ADDR(BW_ADDR),
  .BW_AXI_DATA(BW_AXI_DATA),
  .SPM_SIZE(SPM_SIZE)
)
i_local_lsu_with_spm
(
	.clk(clk),
	.rstnn(rstnn),

	.rreg_sme_list(rreg_sme_list),
	.rreg_smvalue_list(rreg_smvalue_list),
	.wreg_rme_list(wreg_rme_list),
	.wreg_rmvalue_list(wreg_rmvalue_list),

	.start(start),
	.finish(finish),

	.subop(subop),
	.immediate_value(immediate_value),

  .core0_rbqhint(core0_rbqhint),
  .core0_rbqvalid(core0_rbqvalid),
  .core0_rbqready(core0_rbqready),
  .core0_rbqwrite(core0_rbqwrite),
  .core0_rbqaddr(core0_rbqaddr),
  .core0_rbqwvalue(core0_rbqwvalue),
  .core0_rbqwstrb(core0_rbqwstrb),
  .core0_rbpvalid(core0_rbpvalid),
  .core0_rbpready(core0_rbpready),
  .core0_rbprvalue(core0_rbprvalue),

	.external_rbqhint(external_bqhint),
  .external_rbqvalid(external_bqvalid),
  .external_rbqready(external_bqready),
  .external_rbqwrite(external_bqwrite),
  .external_rbqaddr(external_bqaddr),
  .external_rbqwvalue(external_bqwvalue),
  .external_rbqwstrb(external_bqwstrb),
  .external_rbpvalid(external_bpvalid),
  .external_rbpready(external_bpready),
  .external_rbprvalue(external_bprvalue)
);

endmodule
