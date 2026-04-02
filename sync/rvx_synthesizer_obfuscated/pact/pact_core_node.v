`include "ervp_global.vh"
`include "ervp_axi_define.vh"

`include "pact_extended_config.vh"
`include "pact_memorymap_offset.vh"

module PACT_CORE_NODE
(
	clk,
	rstnn,
  enable,

	start,
	finish,
	subop,
	operation_is_float,
	immediate_value,

  direct_sxarid,
	direct_sxaraddr,
	direct_sxarlen,
	direct_sxarsize,
	direct_sxarburst,
	direct_sxarvalid,
	direct_sxarready,

	direct_sxrid,
	direct_sxrdata,
	direct_sxrresp,
	direct_sxrlast,
	direct_sxrvalid,
	direct_sxrready,

	direct_sxawid,
  direct_sxawaddr,
  direct_sxawlen,
  direct_sxawsize,
  direct_sxawburst,
  direct_sxawvalid,
  direct_sxawready,

  direct_sxwid,
  direct_sxwdata,
  direct_sxwstrb,
  direct_sxwlast,
  direct_sxwvalid,
  direct_sxwready,

  direct_sxbid,
  direct_sxbresp,
  direct_sxbvalid,
  direct_sxbready,

  corecache_sxarid,
	corecache_sxaraddr,
	corecache_sxarlen,
	corecache_sxarsize,
	corecache_sxarburst,
	corecache_sxarvalid,
	corecache_sxarready,

	corecache_sxrid,
	corecache_sxrdata,
	corecache_sxrresp,
	corecache_sxrlast,
	corecache_sxrvalid,
	corecache_sxrready,

	corecache_sxawid,
  corecache_sxawaddr,
  corecache_sxawlen,
  corecache_sxawsize,
  corecache_sxawburst,
  corecache_sxawvalid,
  corecache_sxawready,

  corecache_sxwid,
  corecache_sxwdata,
  corecache_sxwstrb,
  corecache_sxwlast,
  corecache_sxwvalid,
  corecache_sxwready,

  corecache_sxbid,
  corecache_sxbresp,
  corecache_sxbvalid,
  corecache_sxbready,

  lsu0cache_sbqhint,
  lsu0cache_sbqvalid,
  lsu0cache_sbqready,
  lsu0cache_sbqwrite,
  lsu0cache_sbqaddr,
  lsu0cache_sbqwvalue,
  lsu0cache_sbqwstrb,
  lsu0cache_sbpvalid,
  lsu0cache_sbpready,
  lsu0cache_sbprvalue
);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 32;
parameter BW_AXI_TID = 4;
parameter BW_DATA = 32;
parameter BW_CORECACHE_DATA = 32;
parameter BW_CORECACHE_ACCESS = 32;
parameter BW_LSU0CACHE_ACCESS = 32;

input wire clk, rstnn;
input wire enable;

input wire start;
output reg finish;
input wire [`BW_PACT_SUBOP-1:0] subop;
input wire operation_is_float;
input wire [`BW_PACT_IMMEDIATE-1:0] immediate_value;

output wire [BW_AXI_TID-1:0] direct_sxarid;
output wire [BW_ADDR-1:0] direct_sxaraddr;
output wire [`BW_AXI_ALEN-1:0] direct_sxarlen;
output wire [`BW_AXI_ASIZE-1:0] direct_sxarsize;
output wire [`BW_AXI_ABURST-1:0] direct_sxarburst;
output wire direct_sxarvalid;
input wire direct_sxarready;

input wire [BW_AXI_TID-1:0] direct_sxrid;
input wire [BW_DATA-1:0] direct_sxrdata;
input wire [`BW_AXI_RRESP-1:0] direct_sxrresp;
input wire direct_sxrlast;
input wire direct_sxrvalid;
output wire direct_sxrready;

output wire [BW_AXI_TID-1:0] direct_sxawid;
output wire [BW_ADDR-1:0] direct_sxawaddr;
output wire [`BW_AXI_ALEN-1:0] direct_sxawlen;
output wire [`BW_AXI_ASIZE-1:0] direct_sxawsize;
output wire [`BW_AXI_ABURST-1:0] direct_sxawburst;
output wire direct_sxawvalid;
input wire direct_sxawready;

output wire [BW_AXI_TID-1:0] direct_sxwid;
output wire [BW_DATA-1:0] direct_sxwdata;
output wire [`BW_AXI_WSTRB(BW_DATA)-1:0] direct_sxwstrb;
output wire direct_sxwlast;
output wire direct_sxwvalid;
input wire direct_sxwready;

input wire [BW_AXI_TID-1:0] direct_sxbid;
input wire [`BW_AXI_BRESP-1:0] direct_sxbresp;
input wire direct_sxbvalid;
output wire direct_sxbready;

output wire [BW_AXI_TID-1:0] corecache_sxarid;
output wire [BW_ADDR-1:0] corecache_sxaraddr;
output wire [`BW_AXI_ALEN-1:0] corecache_sxarlen;
output wire [`BW_AXI_ASIZE-1:0] corecache_sxarsize;
output wire [`BW_AXI_ABURST-1:0] corecache_sxarburst;
output wire corecache_sxarvalid;
input wire corecache_sxarready;

input wire [BW_AXI_TID-1:0] corecache_sxrid;
input wire [BW_CORECACHE_DATA-1:0] corecache_sxrdata;
input wire [`BW_AXI_RRESP-1:0] corecache_sxrresp;
input wire corecache_sxrlast;
input wire corecache_sxrvalid;
output wire corecache_sxrready;

output wire [BW_AXI_TID-1:0] corecache_sxawid;
output wire [BW_ADDR-1:0] corecache_sxawaddr;
output wire [`BW_AXI_ALEN-1:0] corecache_sxawlen;
output wire [`BW_AXI_ASIZE-1:0] corecache_sxawsize;
output wire [`BW_AXI_ABURST-1:0] corecache_sxawburst;
output wire corecache_sxawvalid;
input wire corecache_sxawready;

output wire [BW_AXI_TID-1:0] corecache_sxwid;
output wire [BW_CORECACHE_DATA-1:0] corecache_sxwdata;
output wire [`BW_AXI_WSTRB(BW_CORECACHE_DATA)-1:0] corecache_sxwstrb;
output wire corecache_sxwlast;
output wire corecache_sxwvalid;
input wire corecache_sxwready;

input wire [BW_AXI_TID-1:0] corecache_sxbid;
input wire [`BW_AXI_BRESP-1:0] corecache_sxbresp;
input wire corecache_sxbvalid;
output wire corecache_sxbready;

output wire lsu0cache_sbqhint;
output wire lsu0cache_sbqvalid;
input wire lsu0cache_sbqready;
output wire lsu0cache_sbqwrite;
output wire [BW_ADDR-1:0] lsu0cache_sbqaddr;
output wire [BW_LSU0CACHE_ACCESS-1:0] lsu0cache_sbqwvalue;
output wire [`BW_AXI_WSTRB(BW_LSU0CACHE_ACCESS)-1:0] lsu0cache_sbqwstrb;
input wire lsu0cache_sbpvalid;
output wire lsu0cache_sbpready;
input wire [BW_LSU0CACHE_ACCESS-1:0] lsu0cache_sbprvalue;

/////////////
/* signals */
/////////////

localparam BW_STATE = `BW_PACT_SUBOP_CORE;
localparam IDLE = `PACT_SUBOP_CORE_IDLE;

reg [BW_STATE-1:0] state;
reg [`MAX(BW_ADDR,`BW_PACT_IMMEDIATE)-1:0] immediate_value_stored;

wire i_core_axi_busy;
wire i_core_axi_set_start_addr;
wire [BW_ADDR-1:0] i_core_axi_start_addr;
wire i_core_axi_start;

////////////
/* logics */
////////////

PACT_CORE_AXI
#(
  .BW_ADDR(BW_ADDR),
  .BW_DATA(BW_DATA),
  .BW_AXI_TID(BW_AXI_TID),
  .BW_CORECACHE_DATA(BW_CORECACHE_DATA),
  .BW_CORECACHE_ACCESS(BW_CORECACHE_ACCESS),
  .BW_LSU0CACHE_ACCESS(BW_LSU0CACHE_ACCESS)
)
i_core_axi
(
	.clk(clk),
	.rstnn(rstnn),
  .enable(enable),

  .busy(i_core_axi_busy),
  .set_start_addr(i_core_axi_set_start_addr),
  .start_addr(i_core_axi_start_addr),
  .start(i_core_axi_start),

	.direct_sxarid(direct_sxarid),
	.direct_sxaraddr(direct_sxaraddr),
	.direct_sxarlen(direct_sxarlen),
	.direct_sxarsize(direct_sxarsize),
	.direct_sxarburst(direct_sxarburst),
	.direct_sxarvalid(direct_sxarvalid),
	.direct_sxarready(direct_sxarready),

	.direct_sxrid(direct_sxrid),
	.direct_sxrdata(direct_sxrdata),
	.direct_sxrresp(direct_sxrresp),
	.direct_sxrlast(direct_sxrlast),
	.direct_sxrvalid(direct_sxrvalid),
	.direct_sxrready(direct_sxrready),

	.direct_sxawid(direct_sxawid),
  .direct_sxawaddr(direct_sxawaddr),
  .direct_sxawlen(direct_sxawlen),
  .direct_sxawsize(direct_sxawsize),
  .direct_sxawburst(direct_sxawburst),
  .direct_sxawvalid(direct_sxawvalid),
  .direct_sxawready(direct_sxawready),

  .direct_sxwid(direct_sxwid),
  .direct_sxwdata(direct_sxwdata),
  .direct_sxwstrb(direct_sxwstrb),
  .direct_sxwlast(direct_sxwlast),
  .direct_sxwvalid(direct_sxwvalid),
  .direct_sxwready(direct_sxwready),

  .direct_sxbid(direct_sxbid),
  .direct_sxbresp(direct_sxbresp),
  .direct_sxbvalid(direct_sxbvalid),
  .direct_sxbready(direct_sxbready),

  .corecache_sxarid(corecache_sxarid),
	.corecache_sxaraddr(corecache_sxaraddr),
	.corecache_sxarlen(corecache_sxarlen),
	.corecache_sxarsize(corecache_sxarsize),
	.corecache_sxarburst(corecache_sxarburst),
	.corecache_sxarvalid(corecache_sxarvalid),
	.corecache_sxarready(corecache_sxarready),

	.corecache_sxrid(corecache_sxrid),
	.corecache_sxrdata(corecache_sxrdata),
	.corecache_sxrresp(corecache_sxrresp),
	.corecache_sxrlast(corecache_sxrlast),
	.corecache_sxrvalid(corecache_sxrvalid),
	.corecache_sxrready(corecache_sxrready),

	.corecache_sxawid(corecache_sxawid),
  .corecache_sxawaddr(corecache_sxawaddr),
  .corecache_sxawlen(corecache_sxawlen),
  .corecache_sxawsize(corecache_sxawsize),
  .corecache_sxawburst(corecache_sxawburst),
  .corecache_sxawvalid(corecache_sxawvalid),
  .corecache_sxawready(corecache_sxawready),

  .corecache_sxwid(corecache_sxwid),
  .corecache_sxwdata(corecache_sxwdata),
  .corecache_sxwstrb(corecache_sxwstrb),
  .corecache_sxwlast(corecache_sxwlast),
  .corecache_sxwvalid(corecache_sxwvalid),
  .corecache_sxwready(corecache_sxwready),

  .corecache_sxbid(corecache_sxbid),
  .corecache_sxbresp(corecache_sxbresp),
  .corecache_sxbvalid(corecache_sxbvalid),
  .corecache_sxbready(corecache_sxbready),

  .lsu0cache_sbqhint(lsu0cache_sbqhint),
  .lsu0cache_sbqvalid(lsu0cache_sbqvalid),
  .lsu0cache_sbqready(lsu0cache_sbqready),
  .lsu0cache_sbqwrite(lsu0cache_sbqwrite),
  .lsu0cache_sbqaddr(lsu0cache_sbqaddr),
  .lsu0cache_sbqwvalue(lsu0cache_sbqwvalue),
  .lsu0cache_sbqwstrb(lsu0cache_sbqwstrb),
  .lsu0cache_sbpvalid(lsu0cache_sbpvalid),
  .lsu0cache_sbpready(lsu0cache_sbpready),
  .lsu0cache_sbprvalue(lsu0cache_sbprvalue)
);

assign i_core_axi_set_start_addr = (state==`PACT_SUBOP_CORE_STARTADDR);
assign i_core_axi_start_addr = immediate_value_stored;
assign i_core_axi_start = (state==`PACT_SUBOP_CORE_ACTIVE);

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

always@(*)
begin
  finish = 0;
  case(state)
    `PACT_SUBOP_CORE_STARTADDR,
    `PACT_SUBOP_CORE_ACTIVE:
      finish = 1;
    `PACT_SUBOP_CORE_WAIT:
      finish = ~i_core_axi_busy;
  endcase
end

endmodule
