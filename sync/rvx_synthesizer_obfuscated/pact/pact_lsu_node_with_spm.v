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

	spm_rxarid,
	spm_rxaraddr,
	spm_rxarlen,
	spm_rxarsize,
	spm_rxarburst,
	spm_rxarvalid,
	spm_rxarready,

	spm_rxrid,
	spm_rxrdata,
	spm_rxrresp,
	spm_rxrlast,
	spm_rxrvalid,
	spm_rxrready,

	spm_rxawid,
  spm_rxawaddr,
  spm_rxawlen,
  spm_rxawsize,
  spm_rxawburst,
  spm_rxawvalid,
  spm_rxawready,

  spm_rxwid,
  spm_rxwdata,
  spm_rxwstrb,
  spm_rxwlast,
  spm_rxwvalid,
  spm_rxwready,

  spm_rxbid,
  spm_rxbresp,
  spm_rxbvalid,
  spm_rxbready
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 32;
parameter BW_AXI_DATA = 32;
parameter BW_AXI_TID = 4;
parameter SPM_SIZE = 4096*2;
parameter BASEADDR = 0;

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"
`include "pact_para_matrix.vb"

localparam BW_ACCESS = BW_AXI_DATA;

localparam NUM_BIPUT_REG = 0;
localparam NUM_INPUT_REG = 1;
localparam NUM_OUTPUT_REG = 1;

localparam NUM_READ_REG = 1;
localparam NUM_WRITE_REG = 1;

localparam CELL_SIZE = SPM_SIZE;
localparam CELL_WIDTH = BW_AXI_DATA;
localparam BW_BYTE_WEN = `NUM_BYTE(CELL_WIDTH);
localparam CELL_DEPTH = `DIVIDERU(CELL_SIZE,BW_BYTE_WEN);
localparam BW_CELL_INDEX = BW_ADDR; // LOG2RU(CELL_DEPTH)

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

input wire [BW_AXI_TID-1:0] spm_rxarid;
input wire [BW_ADDR-1:0] spm_rxaraddr;
input wire [`BW_AXI_ALEN-1:0] spm_rxarlen;
input wire [`BW_AXI_ASIZE-1:0] spm_rxarsize;
input wire [`BW_AXI_ABURST-1:0] spm_rxarburst;
input wire spm_rxarvalid;
output wire spm_rxarready;

output wire [BW_AXI_TID-1:0] spm_rxrid;
output wire [BW_AXI_DATA-1:0] spm_rxrdata;
output wire [`BW_AXI_RRESP-1:0] spm_rxrresp;
output wire spm_rxrlast;
output wire spm_rxrvalid;
input wire spm_rxrready;

input wire [BW_AXI_TID-1:0] spm_rxawid;
input wire [BW_ADDR-1:0] spm_rxawaddr;
input wire [`BW_AXI_ALEN-1:0] spm_rxawlen;
input wire [`BW_AXI_ASIZE-1:0] spm_rxawsize;
input wire [`BW_AXI_ABURST-1:0] spm_rxawburst;
input wire spm_rxawvalid;
output wire spm_rxawready;

input wire [BW_AXI_TID-1:0] spm_rxwid;
input wire [BW_AXI_DATA-1:0] spm_rxwdata;
input wire [`BW_AXI_WSTRB(BW_AXI_DATA)-1:0] spm_rxwstrb;
input wire spm_rxwlast;
input wire spm_rxwvalid;
output wire spm_rxwready;

output wire [BW_AXI_TID-1:0] spm_rxbid;
output wire [`BW_AXI_BRESP-1:0] spm_rxbresp;
output wire spm_rxbvalid;
input wire spm_rxbready;

/////////////
/* signals */
/////////////

wire [2-1:0] spm_lqdready;
wire spm_lqvalid;
wire spm_lqhint;
wire spm_lqlast;
wire spm_lqafy;
wire [BW_LPI_QDATA-1:0] spm_lqdata;
wire [2-1:0] spm_lydready;
wire spm_lyvalid;
wire spm_lyhint;
wire spm_lylast;
wire [BW_LPI_YDATA-1:0] spm_lydata;

localparam NUM_EXTERNAL_INTERFACE = 2;
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

wire [BW_CELL_INDEX-1:0] pcell_index;
wire pcell_write_enable;
wire [BW_BYTE_WEN-1:0] pcell_write_enable_byte;
wire [CELL_WIDTH-1:0] pcell_wdata;
wire pcell_read_enable;
wire [CELL_WIDTH-1:0] pcell_rdata;

////////////
/* logics */
////////////

MUNOC_AXI2LPIMO
#(
  .BW_ADDR(BW_ADDR),
  .BW_DATA(BW_AXI_DATA),
  .BW_AXI_TID(BW_AXI_TID)
)
i_axi2lpimo
(
	.clk(clk),
	.rstnn(rstnn),
  .clear(1'b 0),
  .enable(1'b 1),

  .rxawid(spm_rxawid),
	.rxawaddr(spm_rxawaddr),
	.rxawlen(spm_rxawlen),
	.rxawsize(spm_rxawsize),
	.rxawburst(spm_rxawburst),
	.rxawvalid(spm_rxawvalid),
	.rxawready(spm_rxawready),

	.rxwid(spm_rxwid),
	.rxwdata(spm_rxwdata),
	.rxwstrb(spm_rxwstrb),
	.rxwlast(spm_rxwlast),
	.rxwvalid(spm_rxwvalid),
	.rxwready(spm_rxwready),

	.rxbid(spm_rxbid),
	.rxbresp(spm_rxbresp),
	.rxbvalid(spm_rxbvalid),
	.rxbready(spm_rxbready),

	.rxarid(spm_rxarid),
	.rxaraddr(spm_rxaraddr),
	.rxarlen(spm_rxarlen),
	.rxarsize(spm_rxarsize),
	.rxarburst(spm_rxarburst),
	.rxarvalid(spm_rxarvalid),
	.rxarready(spm_rxarready),

	.rxrid(spm_rxrid),
	.rxrdata(spm_rxrdata),
	.rxrresp(spm_rxrresp),
	.rxrlast(spm_rxrlast),
	.rxrvalid(spm_rxrvalid),
	.rxrready(spm_rxrready),

  .slqdready(spm_lqdready),
  .slqvalid(spm_lqvalid),
  .slqhint(spm_lqhint),
  .slqlast(spm_lqlast),
  .slqafy(spm_lqafy),
  .slqdata(spm_lqdata),
  .slydready(spm_lydready),
  .slyvalid(spm_lyvalid),
  .slyhint(spm_lyhint),
  .slylast(spm_lylast),
  .slydata(spm_lydata)
);

assign {core0_rlqdready, spm_lqdready} = external_lqdready_list;
assign external_lqvalid_list = {core0_rlqvalid, spm_lqvalid};
assign external_lqhint_list = {core0_rlqhint, spm_lqhint};
assign external_lqlast_list = {core0_rlqlast, spm_lqlast};
assign external_lqafy_list = {core0_rlqafy, spm_lqafy};
assign external_lqdata_list = {core0_rlqdata, spm_lqdata};

assign external_lydready_list = {core0_rlydready, spm_lydready};
assign {core0_rlyvalid, spm_lyvalid} = external_lyvalid_list;
assign {core0_rlyhint, spm_lyhint} = external_lyhint_list;
assign {core0_rlylast, spm_lylast} = external_lylast_list;
assign {core0_rlydata, spm_lydata} = external_lydata_list;

PACT_LSU
#(
  .BW_ADDR(BW_ADDR),
  .BW_ACCESS(BW_ACCESS),
  .NUM_EXTERNAL_INTERFACE(NUM_EXTERNAL_INTERFACE),
  .BASEADDR(BASEADDR)
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

  .cache_control_valid(),
  .cache_control_ready(1'b 1),
  .cache_control_command(),
  .cache_control_base(),
  .cache_control_last(),

  .cache_busy(1'b 0),
  .cache_control_busy(1'b 0),

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

MUNOC_LPIMO2SCELL
#(
  .BW_ADDR(BW_ADDR),
  .BW_DATA(BW_ACCESS),
  .BASEADDR(BASEADDR),
  .BW_CELL_INDEX(BW_CELL_INDEX),
  .CELL_WIDTH(CELL_WIDTH),
  .NUM_CELL(1),
  .BW_LPI_BURDEN(ACCESS_BW_LPI_BURDEN)
)
i_lpimo2scell
(
	.clk(clk),
	.rstnn(rstnn),
  .clear(1'b 0),
  .enable(1'b 1),

  .rlqdready(access_lqdready),
  .rlqvalid(access_lqvalid),
  .rlqhint(access_lqhint),
  .rlqlast(access_lqlast),
  .rlqafy(access_lqafy),
  .rlqdata(access_lqdata),
  .rlydready(access_lydready),
  .rlyvalid(access_lyvalid),
  .rlyhint(access_lyhint),
  .rlylast(access_lylast),
  .rlydata(access_lydata),

  .sscell_select_list(),
	.sscell_index_list(pcell_index),
	.sscell_enable_list(),
	.sscell_wenable_list(pcell_write_enable),
	.sscell_wenable_byte_list(pcell_write_enable_byte),
	.sscell_wenable_bit_list(),
	.sscell_wdata_list(pcell_wdata),
	.sscell_renable_list(pcell_read_enable),
	.sscell_rdata_list(pcell_rdata)
);

ERVP_MEMORY_CELL_1R1W
#(
  .DEPTH(CELL_DEPTH),
  .WIDTH(CELL_WIDTH),
  .BW_INDEX(BW_CELL_INDEX),
  .USE_SINGLE_INDEX(1),
  .USE_SUBWORD_ENABLE(1),
  .BW_SUBWORD(8)
)
i_cell
(
  .clk(clk),
  .rstnn(rstnn),
  .index(pcell_index),
  .windex(pcell_index),
  .wenable(pcell_write_enable),
  .wpermit(pcell_write_enable_byte),
  .wdata(pcell_wdata),
  .rindex(pcell_index),
  .rdata_asynch(),
  .renable(pcell_read_enable),
  .rdata_synch(pcell_rdata)
);

endmodule
