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
wire [NUM_CELL-1:0] cell_stall_list;

wire [BW_CELL_INDEX-1:0] cell_index [NUM_CELL-1:0];
wire cell_enable [NUM_CELL-1:0];
wire cell_wenable [NUM_CELL-1:0];
wire [BW_BYTE_WEN-1:0] cell_wenable_byte [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_wenable_bit [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_wdata [NUM_CELL-1:0];
wire cell_renable [NUM_CELL-1:0];
wire [BW_DATA-1:0] cell_rdata [NUM_CELL-1:0];

localparam BW_STATE = BW_CONFIG;

reg [BW_STATE-1:0] cell_state;
wire is_last_cell_state;

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
  assign cell_stall_list[i] = ~is_last_cell_state;
end
endgenerate

localparam [64-1:0] WRITE_RECOVERY_TIME_NS = 12;
localparam [32-1:0] WRITE_RECOVERY_CYCLE = ((WRITE_RECOVERY_TIME_NS-1)*CLK_HZ/1000000000) + 1;

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
    cell_state <= 1;
  else if(cell_enable[0])
  begin
    if(is_last_cell_state)
    begin
      if(cell_renable[0])
        cell_state <= (1 << WRITE_RECOVERY_CYCLE);
      else
        cell_state <= 1;
    end
    else
      cell_state <= {cell_state,cell_state[BW_STATE-1]};
  end
end

assign is_last_cell_state = cell_state[BW_STATE-1] | (cell_state==control_rmx_core_config);

`ifndef SIMULATE_EXT_MRAM_BEHAVIOR

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

`else // SIMULATE_EXT_MRAM_BEHAVIOR

reg [(21)-1:0] extmr_a_reg;
reg extmr_e_n_reg;
reg extmr_w_n_reg;
reg extmr_g_n_reg;
reg [(4)-1:0] extmr_be_n_reg;
reg [(32)-1:0] extmr_dq_sod_reg;
reg [(32)-1:0] extmr_wdata;
reg [(32)-1:0] extmr_rdata;

always@(posedge clk, negedge rstnn)
begin
  if(rstnn==0)
  begin
    extmr_a_reg <= 0;
    extmr_e_n_reg <= 1;
    extmr_w_n_reg <= 1;
    extmr_g_n_reg <= 1;
    extmr_be_n_reg <= -1;
    extmr_dq_sod_reg <= -1;
    extmr_wdata <= 0;
    extmr_rdata <= 0;
  end
  else if(cell_enable[0])
  begin
    if(is_last_cell_state)
    begin
      extmr_e_n_reg <= 1;
      extmr_w_n_reg <= 1;
      extmr_g_n_reg <= 1;
      extmr_be_n_reg <= -1;
      // extmr_dq_sod_reg <= -1;
      if(cell_renable[0])
        extmr_rdata <= EXTMR_DQ_sival;
    end
    else if(cell_state[WRITE_RECOVERY_CYCLE])
    begin
      extmr_a_reg <= cell_index[0];
      extmr_e_n_reg <= 0;
      extmr_w_n_reg <= ~cell_wenable[0];
      extmr_g_n_reg <= ~cell_renable[0];
      extmr_dq_sod_reg <= ~cell_wenable_bit[0];
      if(cell_wenable[0])
        extmr_wdata <= cell_wdata[0];
    end
    else if(cell_state[WRITE_RECOVERY_CYCLE+1])
    begin
      extmr_be_n_reg <= (cell_renable[0])? 0 : ~cell_wenable_byte[0];
    end
  end
end

assign EXTMR_A = extmr_a_reg;
assign EXTMR_E_N = extmr_e_n_reg;
assign EXTMR_W_N = extmr_w_n_reg;
assign EXTMR_G_N = extmr_g_n_reg;
assign EXTMR_BE_N = extmr_be_n_reg;
assign EXTMR_DQ_sod = extmr_dq_sod_reg;
assign EXTMR_LS_OE_P = ~extmr_w_n_reg;
assign EXTMR_LS_CE_N = 0;

assign EXTMR_DQ_soval = extmr_wdata;
assign cell_rdata[0] = extmr_rdata;

`endif // SIMULATE_EXT_MRAM_BEHAVIOR
