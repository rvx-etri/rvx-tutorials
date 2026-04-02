/////////////
/* signals */
/////////////

genvar i;

`include "lpit_function.vb"
`include "lpixm_function.vb"

localparam BW_LPIXM_ADDR = BW_AXI_ADDR;
localparam BW_LPIXM_DATA = BW_AXI_DATA;

`include "lpixm_lpara.vb"

wire [2-1:0] lqdready;
wire lqvalid;
wire lqhint;
wire lqlast;
wire lqafy;
wire [BW_LPI_QDATA-1:0] lqdata;

wire [2-1:0] lydready;
wire lyvalid;
wire lyhint;
wire lylast;
wire [BW_LPI_YDATA-1:0] lydata;

wire [NUM_CELL-1:0] cell_select_list;
wire [BW_CELL_INDEX*NUM_CELL-1:0] cell_index_list;
wire [NUM_CELL-1:0] cell_enable_list;
wire [NUM_CELL-1:0] cell_wenable_list;
wire [BW_BYTE_WEN*NUM_CELL-1:0] cell_wenable_byte_list;
wire [BW_DATA*NUM_CELL-1:0] cell_wenable_bit_list;
wire [BW_DATA*NUM_CELL-1:0] cell_wdata_list;
wire [NUM_CELL-1:0] cell_renable_list;
wire [BW_DATA*NUM_CELL-1:0] cell_rdata_list;
wire [NUM_CELL-1:0] cell_stall_list = 0;

wire [BW_CELL_INDEX-1:0] cell_index [NUM_CELL-1:0];
wire cell_enable [NUM_CELL-1:0];
wire cell_wenable [NUM_CELL-1:0];
wire [BW_BYTE_WEN-1:0] cell_wenable_byte [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_wenable_bit [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_wdata [NUM_CELL-1:0];
wire cell_renable [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_rdata [NUM_CELL-1:0];

////////////
/* logics */
////////////

MUNOC_LPIXM2SCELL
#(
  .BW_ADDR(BW_ADDR),
  .BW_DATA(BW_DATA),
  .BW_LPI_BURDEN(BW_LPI_BURDEN),
  .BASEADDR(0),
  .BW_CELL_INDEX(BW_CELL_INDEX),
  .CELL_WIDTH(CELL_WIDTH),
  .NUM_CELL(NUM_CELL)
)
i_lpixm2scell
(
	.clk(clk),
	.rstnn(rstnn),
  .clear(1'b 0),
  .enable(1'b 1),

  .rlqdready(lqdready),
  .rlqvalid(lqvalid),
  .rlqhint(lqhint),
  .rlqlast(lqlast),
  .rlqafy(lqafy),
  .rlqdata(lqdata),

  .rlydready(lydready),
  .rlyvalid(lyvalid),
  .rlyhint(lyhint),
  .rlylast(lylast),
  .rlydata(lydata),

  .sscell_select_list(cell_select_list),
	.sscell_index_list(cell_index_list),
	.sscell_enable_list(cell_enable_list),
	.sscell_wenable_list(cell_wenable_list),
	.sscell_wenable_byte_list(cell_wenable_byte_list),
	.sscell_wenable_bit_list(cell_wenable_bit_list),
	.sscell_wdata_list(cell_wdata_list),
	.sscell_renable_list(cell_renable_list),
	.sscell_rdata_list(cell_rdata_list),
	.sscell_stall_list(cell_stall_list)
);

assign rlxqdready = lqdready;
assign lqvalid = rlxqvalid;
assign lqhint = 0;
assign lqlast = rlxqlast;
assign lqafy = (~rlxqwrite) | rlxqlast;
assign lqdata = {rlxqburden,rlxqwrite,rlxqlen,rlxqsize,rlxqburst,rlxqwstrb,rlxqwdata,rlxqaddr};

assign lydready = rlxydready;
assign rlxyvalid = lyvalid;
assign rlxylast = lylast;
assign {rlxyburden,rlxywreply,rlxyresp,rlxyrdata} = lydata;

generate
for(i=0; i<NUM_CELL; i=i+1)
begin : generate_cell_signals
  assign cell_index[i] = cell_index_list[BW_CELL_INDEX*(i+1)-1 -:BW_CELL_INDEX];
  assign cell_enable[i] = cell_enable_list[i];
  assign cell_wenable[i] = cell_wenable_list[i];
  assign cell_wenable_byte[i] = cell_wenable_byte_list[BW_BYTE_WEN*(i+1)-1 -:BW_BYTE_WEN];
  assign cell_wenable_bit[i] = cell_wenable_bit_list[BW_DATA*(i+1)-1 -:BW_DATA];
  assign cell_wdata[i] = cell_wdata_list[BW_DATA*(i+1)-1 -:BW_DATA];
  assign cell_renable[i] = cell_renable_list[i];
  assign cell_rdata_list[BW_DATA*(i+1)-1 -:BW_DATA] = cell_rdata[i];
end
endgenerate

generate
for(i=0; i<NUM_CELL; i=i+1)
begin : generate_cell
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
    .index(cell_index[i]),
    .windex(cell_index[i]),
    .wenable(cell_wenable[i]),
    .wpermit(cell_wenable_byte[i]),
    .wdata(cell_wdata[i]),
    .rindex(cell_index[i]),
    .rdata_asynch(),
    .renable(cell_renable[i]),
    .rdata_synch(cell_rdata[i])
  );
end
endgenerate
