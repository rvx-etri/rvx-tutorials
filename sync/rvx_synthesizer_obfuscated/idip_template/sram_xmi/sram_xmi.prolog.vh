`include "ervp_global.vh"
`include "ervp_axi_define.vh"
`include "munoc_network_include.vh"

module "${MODULE_NAME}"
(
	clk,
	rstnn,

	rlxqdready,
  rlxqvalid,
  rlxqlast,
  rlxqwrite,
  rlxqlen,
  rlxqsize,
  rlxqburst,
  rlxqwstrb,
  rlxqwdata,
  rlxqaddr,
  rlxqburden,
  rlxydready,
  rlxyvalid,
  rlxylast,
  rlxywreply,
  rlxyresp,
  rlxyrdata,
  rlxyburden
);

////////////////////////////
/* parameter input output */
////////////////////////////

localparam CAPACITY = "${CAPACITY}";  // in bytes
localparam BW_ADDR = "${BW_ADDR}";
localparam BW_DATA = "${BW_DATA}";
localparam BW_LPI_BURDEN = `REQUIRED_BW_OF_SLAVE_TID;
localparam CELL_SIZE = "${CELL_SIZE}";  // in bytes
localparam CELL_WIDTH = "${CELL_WIDTH}"; // MUST greater than or equal to BW_DATA

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

localparam BW_BYTE_WEN = `NUM_BYTE(CELL_WIDTH);
localparam CELL_DEPTH = `DIVIDERU(CELL_SIZE,BW_BYTE_WEN);
localparam BW_CELL_INDEX = REQUIRED_BITWIDTH_INDEX(CELL_DEPTH);

localparam NUM_CELL = `DIVIDERU(CAPACITY,CELL_SIZE);

input wire clk, rstnn;

localparam BW_AXI_ADDR = BW_ADDR;
localparam BW_AXI_DATA = BW_DATA;
localparam BW_AXI_TID = BW_LPI_BURDEN;

output wire [(2)-1:0] rlxqdready;
input wire rlxqvalid;
input wire rlxqlast;
input wire rlxqwrite;
input wire [`BW_AXI_ALEN-1:0] rlxqlen;
input wire [`BW_AXI_ASIZE-1:0] rlxqsize;
input wire [`BW_AXI_ABURST-1:0] rlxqburst;
input wire [`BW_AXI_WSTRB(BW_AXI_DATA)-1:0] rlxqwstrb;
input wire [BW_AXI_DATA-1:0] rlxqwdata;
input wire [BW_AXI_ADDR-1:0] rlxqaddr;
input wire [BW_LPI_BURDEN-1:0] rlxqburden;
input wire [(2)-1:0] rlxydready;
output wire rlxyvalid;
output wire rlxylast;
output wire rlxywreply;
output wire [`BW_AXI_RESP-1:0] rlxyresp;
output wire [BW_AXI_DATA-1:0] rlxyrdata;
output wire [BW_LPI_BURDEN-1:0] rlxyburden;
