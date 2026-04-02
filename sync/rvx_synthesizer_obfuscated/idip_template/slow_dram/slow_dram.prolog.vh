`include "ervp_global.vh"
`include "ervp_axi_define.vh"

module "${MODULE_NAME}"
(
	clk_ref,
	clk_sys,
	rstnn_sys,
	rstnn_dram_if,
	
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
	rxrready,

	clk_dram_if,
	initialized
	`include "slow_dram_cell_port_dec.vh"
);

////////////////////////////
/* parameter input output */
////////////////////////////

localparam BW_ADDR = 32;
localparam BW_DATA = 32;
localparam BW_AXI_TID = 16;

input wire clk_ref;
input wire clk_sys;
input wire rstnn_sys;
input wire rstnn_dram_if;

input wire rxrready;
output wire rxrvalid;
output wire rxrlast;
output wire [`BW_AXI_RRESP-1:0] rxrresp;
output wire [BW_DATA-1:0] rxrdata;
output wire [BW_AXI_TID-1:0] rxrid;
output wire rxarready;
input wire rxarvalid;
input wire [`BW_AXI_ABURST-1:0] rxarburst;
input wire [`BW_AXI_ASIZE-1:0] rxarsize;
input wire [`BW_AXI_ALEN-1:0] rxarlen;
input wire [BW_ADDR-1:0] rxaraddr;
input wire [BW_AXI_TID-1:0] rxarid;
input wire rxbready;
output wire rxbvalid;
output wire [`BW_AXI_BRESP-1:0] rxbresp;
output wire [BW_AXI_TID-1:0] rxbid;
output wire rxwready;
input wire rxwvalid;
input wire rxwlast;
input wire [`BW_AXI_WSTRB(BW_DATA)-1:0] rxwstrb;
input wire [BW_DATA-1:0] rxwdata;
input wire [BW_AXI_TID-1:0] rxwid;
output wire rxawready;
input wire rxawvalid;
input wire [`BW_AXI_ABURST-1:0] rxawburst;
input wire [`BW_AXI_ASIZE-1:0] rxawsize;
input wire [`BW_AXI_ALEN-1:0] rxawlen;
input wire [BW_ADDR-1:0] rxawaddr;
input wire [BW_AXI_TID-1:0] rxawid;

output wire clk_dram_if;
output wire initialized;

`include "slow_dram_cell_port_def.vh"
