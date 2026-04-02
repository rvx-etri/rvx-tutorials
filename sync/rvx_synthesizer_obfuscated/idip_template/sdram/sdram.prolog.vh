`include "ervp_global.vh"
`include "ervp_axi_define.vh"

module "${MODULE_NAME}"
(
	rstnn,
	clk_cell,
	clk_axi,
	clk_apb,

	memory_rxawid,
	memory_rxawaddr,
	memory_rxawlen,
	memory_rxawsize,
	memory_rxawburst,
	memory_rxawvalid,
	memory_rxawready,
	memory_rxwid,
	memory_rxwdata,
	memory_rxwstrb,
	memory_rxwlast,
	memory_rxwvalid,
	memory_rxwready,
	memory_rxbid,
	memory_rxbresp,
	memory_rxbvalid,
	memory_rxbready,
	memory_rxarid,
	memory_rxaraddr,
	memory_rxarlen,
	memory_rxarsize,
	memory_rxarburst,
	memory_rxarvalid,
	memory_rxarready,
	memory_rxrid,
	memory_rxrdata,
	memory_rxrresp,
	memory_rxrlast,
	memory_rxrvalid,
	memory_rxrready,

	ctrl_rpsel,
	ctrl_rpenable,
	ctrl_rpaddr,
	ctrl_rpwrite,
	ctrl_rpwdata,
	ctrl_rprdata,
	ctrl_rpready,
	ctrl_rpslverr,

  LPSDR_DQ_sod,
	LPSDR_DQ_soval,
	LPSDR_DQ_sival,
	LPSDR_DQ_sod_byte,
	LPSDR_DQ_soe_byte
	`include "sdram_cell_port_dec.vh"
);

////////////////////////////
/* parameter input output */
////////////////////////////

localparam BW_ADDR = 32;
localparam BW_AXI_TID = 16;
localparam BW_APB_DATA = 32;
localparam BW_MEMORY_DATA = 32;

input wire rstnn;
input wire clk_cell;
input wire clk_axi;
input wire clk_apb;

input wire [BW_AXI_TID-1:0] memory_rxawid;
input wire [BW_ADDR-1:0] memory_rxawaddr;
input wire [`BW_AXI_ALEN-1:0] memory_rxawlen;
input wire [`BW_AXI_ASIZE-1:0] memory_rxawsize;
input wire [`BW_AXI_ABURST-1:0] memory_rxawburst;
input wire memory_rxawvalid;
output wire memory_rxawready;
input wire [BW_AXI_TID-1:0] memory_rxwid;
input wire [BW_MEMORY_DATA-1:0] memory_rxwdata;
input wire [`BW_AXI_WSTRB(BW_MEMORY_DATA)-1:0] memory_rxwstrb;
input wire memory_rxwlast;
input wire memory_rxwvalid;
output wire memory_rxwready;
output wire [BW_AXI_TID-1:0] memory_rxbid;
output wire [`BW_AXI_BRESP-1:0] memory_rxbresp;
output wire memory_rxbvalid;
input wire memory_rxbready;
input wire [BW_AXI_TID-1:0] memory_rxarid;
input wire [BW_ADDR-1:0] memory_rxaraddr;
input wire [`BW_AXI_ALEN-1:0] memory_rxarlen;
input wire [`BW_AXI_ASIZE-1:0] memory_rxarsize;
input wire [`BW_AXI_ABURST-1:0] memory_rxarburst;
input wire memory_rxarvalid;
output wire memory_rxarready;
output wire [BW_AXI_TID-1:0] memory_rxrid;
output wire [BW_MEMORY_DATA-1:0] memory_rxrdata;
output wire [`BW_AXI_RRESP-1:0] memory_rxrresp;
output wire memory_rxrlast;
output wire memory_rxrvalid;
input wire memory_rxrready;

input wire ctrl_rpsel;
input wire ctrl_rpenable;
input wire [BW_ADDR-1:0] ctrl_rpaddr;
input wire ctrl_rpwrite;
input wire [BW_APB_DATA-1:0] ctrl_rpwdata;
output wire [BW_APB_DATA-1:0] ctrl_rprdata;
output wire ctrl_rpready;
output wire ctrl_rpslverr;

output wire [BW_MEMORY_DATA-1:0] LPSDR_DQ_sod;
output wire [BW_MEMORY_DATA-1:0] LPSDR_DQ_soval;
input wire [BW_MEMORY_DATA-1:0] LPSDR_DQ_sival;
output wire [`NUM_BYTE(BW_MEMORY_DATA)-1:0] LPSDR_DQ_sod_byte;
output wire [`NUM_BYTE(BW_MEMORY_DATA)-1:0] LPSDR_DQ_soe_byte;

`include "sdram_cell_port_def.vh"
