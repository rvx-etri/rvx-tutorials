`include "ervp_global.vh"
`include "ervp_axi_define.vh"
`include "hw_info.vh"
`include "sim_info.vh"
`include "munoc_network_include.vh"

module "${MODULE_NAME}"
(
	clk,
	rstnn,

  control_rmx_core_config,
  control_rmx_core_status,
  control_rmx_clear_request,
  control_rmx_clear_finish,
  control_rmx_log_fifo_wready,
  control_rmx_log_fifo_wrequest,
  control_rmx_log_fifo_wdata,
  control_rmx_inst_fifo_rready,
  control_rmx_inst_fifo_rdata,
  control_rmx_inst_fifo_rrequest,
  control_rmx_operation_finish,
  control_rmx_input_fifo_rready,
  control_rmx_input_fifo_rdata,
  control_rmx_input_fifo_rrequest,
  control_rmx_output_fifo_wready,
  control_rmx_output_fifo_wrequest,
  control_rmx_output_fifo_wdata,

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
  rlxyburden,

  EXTMR_E_N,
  EXTMR_W_N,
  EXTMR_G_N,
  EXTMR_BE_N,
  EXTMR_A,
  EXTMR_DQ_sod,
  EXTMR_DQ_soval,
  EXTMR_DQ_sival,
  EXTMR_LS_OE_P,
  EXTMR_LS_CE_N
);

////////////////////////////
/* parameter input output */
////////////////////////////

localparam CAPACITY = 32'h 8000000;  // in bytes
localparam BW_ADDR = "${BW_ADDR}";
localparam BW_DATA = 32;
localparam BW_LPI_BURDEN = `REQUIRED_BW_OF_SLAVE_TID;
localparam CELL_SIZE = CAPACITY;  // in bytes
localparam CELL_WIDTH = 32; // MUST greater than or equal to BW_DATA
localparam CLK_HZ = "${CLK_HZ}";

`include "ervp_log_util.vf"
`include "ervp_bitwidth_util.vf"

localparam BW_CONFIG = 16;
localparam BW_STATUS = 1;
localparam BW_LOG = 1;
localparam BW_INST = 1;
localparam BW_INPUT = 1;
localparam BW_OUTPUT = 1;

input wire [(BW_CONFIG)-1:0] control_rmx_core_config;
output wire [(BW_STATUS)-1:0] control_rmx_core_status;
input wire control_rmx_clear_request;
output wire control_rmx_clear_finish;
input wire control_rmx_log_fifo_wready;
output wire control_rmx_log_fifo_wrequest;
output wire [(BW_LOG)-1:0] control_rmx_log_fifo_wdata;
input wire control_rmx_inst_fifo_rready;
input wire [(BW_INST)-1:0] control_rmx_inst_fifo_rdata;
output wire control_rmx_inst_fifo_rrequest;
output wire control_rmx_operation_finish;
input wire control_rmx_input_fifo_rready;
input wire [(BW_INPUT)-1:0] control_rmx_input_fifo_rdata;
output wire control_rmx_input_fifo_rrequest;
input wire control_rmx_output_fifo_wready;
output wire control_rmx_output_fifo_wrequest;
output wire [(BW_OUTPUT)-1:0] control_rmx_output_fifo_wdata;

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

output wire EXTMR_E_N;
output wire EXTMR_W_N;
output wire EXTMR_G_N;
output wire [(4)-1:0] EXTMR_BE_N;
output wire [(21)-1:0] EXTMR_A;
output wire [(32)-1:0] EXTMR_DQ_sod;
output wire [(32)-1:0] EXTMR_DQ_soval;
input wire [(32)-1:0] EXTMR_DQ_sival;
output wire EXTMR_LS_OE_P;
output wire EXTMR_LS_CE_N;


assign control_rmx_core_status = 0;
assign control_rmx_clear_finish = 0;
assign control_rmx_log_fifo_wrequest = 0;
assign control_rmx_log_fifo_wdata = 0;
assign control_rmx_inst_fifo_rrequest = 0;
assign control_rmx_operation_finish = 0;
assign control_rmx_input_fifo_rrequest = 0;
assign control_rmx_output_fifo_wrequest = 0;
assign control_rmx_output_fifo_wdata = 0;
