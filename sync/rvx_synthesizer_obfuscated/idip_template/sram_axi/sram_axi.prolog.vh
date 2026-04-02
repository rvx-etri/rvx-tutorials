`include "ervp_global.vh"
`include "ervp_axi_define.vh"
`include "munoc_network_include.vh"

module "${MODULE_NAME}"
(
	clk,
	rstnn,

	rxawid,
	rxawaddr,
	rxawlen,
	rxawsize,
	rxawburst,
	rxawvalid,
	rxawready,

	rxwid,
	rxwdata,
	rxwstrb,
	rxwlast,
	rxwvalid,
	rxwready,

	rxbid,
	rxbresp,
	rxbvalid,
	rxbready,

	rxarid,
	rxaraddr,
	rxarlen,
	rxarsize,
	rxarburst,
	rxarvalid,
	rxarready,

	rxrid,
	rxrdata,
	rxrresp,
	rxrlast,
	rxrvalid,
	rxrready
);

////////////////////////////
/* parameter input output */
////////////////////////////

localparam CAPACITY = "${CAPACITY}";  // in bytes
localparam BW_ADDR = "${BW_ADDR}";
localparam BW_DATA = "${BW_DATA}";
localparam BW_AXI_TID = "${BW_AXI_TID}";
localparam CELL_SIZE = "${CELL_SIZE}";  // in bytes
localparam CELL_WIDTH = "${CELL_WIDTH}"; // MUST greater than or equal to BW_DATA

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

localparam BW_BYTE_WEN = `NUM_BYTE(CELL_WIDTH);
localparam CELL_DEPTH = `DIVIDERU(CELL_SIZE,BW_BYTE_WEN);
localparam BW_CELL_INDEX = REQUIRED_BITWIDTH_INDEX(CELL_DEPTH);

localparam NUM_CELL = `DIVIDERU(CAPACITY,CELL_SIZE);

input wire clk, rstnn;

//AW Channel
input wire [BW_AXI_TID-1:0] rxawid;
input wire [BW_ADDR-1:0] rxawaddr;
input wire [`BW_AXI_ALEN-1:0] rxawlen;
input wire [`BW_AXI_ASIZE-1:0] rxawsize;
input wire [`BW_AXI_ABURST-1:0] rxawburst;
input wire rxawvalid;
output wire rxawready;

//W Channel
input wire [BW_AXI_TID-1:0] rxwid;
input wire [BW_DATA-1:0] rxwdata;
input wire [`BW_AXI_WSTRB(BW_DATA)-1:0] rxwstrb;
input wire rxwlast;
input wire rxwvalid;
output wire rxwready;

//B Channel
output wire [BW_AXI_TID-1:0] rxbid;
output wire [`BW_AXI_BRESP-1:0] rxbresp;
output wire rxbvalid;
input wire rxbready;

//AR Channel
input wire [BW_AXI_TID-1:0] rxarid;
input wire [BW_ADDR-1:0] rxaraddr;
input wire [`BW_AXI_ALEN-1:0] rxarlen;
input wire [`BW_AXI_ASIZE-1:0] rxarsize;
input wire [`BW_AXI_ABURST-1:0] rxarburst;
input wire rxarvalid;
output wire rxarready;

//R Channel
output wire [BW_AXI_TID-1:0] rxrid;
output wire [BW_DATA-1:0] rxrdata;
output wire [`BW_AXI_RRESP-1:0] rxrresp;
output wire rxrlast;
output wire rxrvalid;
input wire rxrready;
