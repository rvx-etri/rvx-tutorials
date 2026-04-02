`ifndef SIMULATE_SDRAM_BEHAVIOR

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

localparam BW_DATA = BW_MEMORY_DATA;
localparam CELL_SIZE = 32'h 8000000;
localparam CELL_WIDTH = BW_DATA;

localparam BW_BYTE_WEN = `NUM_BYTE(CELL_WIDTH);
localparam CELL_DEPTH = `DIVIDERU(CELL_SIZE,BW_BYTE_WEN);
localparam BW_CELL_INDEX = REQUIRED_BITWIDTH_INDEX(CELL_DEPTH);

localparam NUM_CELL = `DIVIDERU(CELL_SIZE,CELL_SIZE);

/////////////
/* signals */
/////////////

genvar i;

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

wire clk;
reg [BW_APB_DATA-1:0] sdram_ctrl;

////////////
/* logics */
////////////

ERVP_SPSRAM_CONTROLLER_AXI
#(
	.BW_ADDR(BW_ADDR),
	.BW_DATA(BW_DATA),
	.BW_AXI_TID(BW_AXI_TID),
	.BASEADDR(0),
	.CELL_SIZE(CELL_SIZE),
  .CELL_WIDTH(CELL_WIDTH),
	.NUM_CELL(NUM_CELL)
)
i_controller
(
	.clk(clk),
	.rstnn(rstnn),
  .enable(1'b 1),

	.rxawid(memory_rxawid),
	.rxawaddr(memory_rxawaddr),
	.rxawlen(memory_rxawlen),
	.rxawsize(memory_rxawsize),
	.rxawburst(memory_rxawburst),
	.rxawvalid(memory_rxawvalid),
	.rxawready(memory_rxawready),

	.rxwid(memory_rxwid),
	.rxwdata(memory_rxwdata),
	.rxwstrb(memory_rxwstrb),
	.rxwlast(memory_rxwlast),
	.rxwvalid(memory_rxwvalid),
	.rxwready(memory_rxwready),

	.rxbid(memory_rxbid),
	.rxbresp(memory_rxbresp),
	.rxbvalid(memory_rxbvalid),
	.rxbready(memory_rxbready),

	.rxarid(memory_rxarid),
	.rxaraddr(memory_rxaraddr),
	.rxarlen(memory_rxarlen),
	.rxarsize(memory_rxarsize),
	.rxarburst(memory_rxarburst),
	.rxarvalid(memory_rxarvalid),
	.rxarready(memory_rxarready),

	.rxrid(memory_rxrid),
	.rxrdata(memory_rxrdata),
	.rxrresp(memory_rxrresp),
	.rxrlast(memory_rxrlast),
	.rxrvalid(memory_rxrvalid),
	.rxrready(memory_rxrready),

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
	.index(cell_index[0]),
	.windex(cell_index[0]),
	.wenable(cell_wenable[0]),
	.wpermit(cell_wenable_byte[0]),
	.wdata(cell_wdata[0]),
	.rindex(cell_index[0]),
	.rdata_asynch(),
	.renable(cell_renable[0]),
	.rdata_synch(cell_rdata[0])
);

assign clk = clk_axi;

always@(posedge clk_apb, negedge rstnn)
begin
	if(rstnn==0)
    sdram_ctrl <= 0;
  else if(ctrl_rpsel && ctrl_rpenable && ctrl_rpwrite)
    sdram_ctrl <= ctrl_rpwdata;
end

assign ctrl_rprdata = sdram_ctrl;
assign ctrl_rpready = 1;
assign ctrl_rpslverr = 0;

`else // SIMULATE_SDRAM_BEHAVIOR

ERVP_SDRAM_CONTROLLER
#(
  .BW_ADDR(BW_ADDR),
  .BW_AXI_TID(BW_AXI_TID)
)
i_controller
(
	.rstnn(rstnn),
	.clk_cell(clk_cell),
	.clk_axi(clk_axi),
	.clk_apb(clk_apb),

	.memory_rxawid(memory_rxawid),
	.memory_rxawaddr(memory_rxawaddr),
	.memory_rxawlen(memory_rxawlen),
	.memory_rxawsize(memory_rxawsize),
	.memory_rxawburst(memory_rxawburst),
	.memory_rxawvalid(memory_rxawvalid),
	.memory_rxawready(memory_rxawready),
	.memory_rxwid(memory_rxwid),
	.memory_rxwdata(memory_rxwdata),
	.memory_rxwstrb(memory_rxwstrb),
	.memory_rxwlast(memory_rxwlast),
	.memory_rxwvalid(memory_rxwvalid),
	.memory_rxwready(memory_rxwready),
	.memory_rxbid(memory_rxbid),
	.memory_rxbresp(memory_rxbresp),
	.memory_rxbvalid(memory_rxbvalid),
	.memory_rxbready(memory_rxbready),
	.memory_rxarid(memory_rxarid),
	.memory_rxaraddr(memory_rxaraddr),
	.memory_rxarlen(memory_rxarlen),
	.memory_rxarsize(memory_rxarsize),
	.memory_rxarburst(memory_rxarburst),
	.memory_rxarvalid(memory_rxarvalid),
	.memory_rxarready(memory_rxarready),
	.memory_rxrid(memory_rxrid),
	.memory_rxrdata(memory_rxrdata),
	.memory_rxrresp(memory_rxrresp),
	.memory_rxrlast(memory_rxrlast),
	.memory_rxrvalid(memory_rxrvalid),
	.memory_rxrready(memory_rxrready),

	.ctrl_rpsel(ctrl_rpsel),
	.ctrl_rpenable(ctrl_rpenable),
	.ctrl_rpaddr(ctrl_rpaddr),
	.ctrl_rpwrite(ctrl_rpwrite),
	.ctrl_rpwdata(ctrl_rpwdata),
	.ctrl_rprdata(ctrl_rprdata),
	.ctrl_rpready(ctrl_rpready),
	.ctrl_rpslverr(ctrl_rpslverr),

	.LPSDR_DQ_sod(LPSDR_DQ_sod),
	.LPSDR_DQ_soval(LPSDR_DQ_soval),
	.LPSDR_DQ_sival(LPSDR_DQ_sival),
	.LPSDR_DQ_sod_byte(LPSDR_DQ_sod_byte),
	.LPSDR_DQ_soe_byte(LPSDR_DQ_soe_byte)

	`include "sdram_cell_port_mapping.vh"
);

`endif
