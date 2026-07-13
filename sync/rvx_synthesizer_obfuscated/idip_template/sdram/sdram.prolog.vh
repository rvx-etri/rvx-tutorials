`include "ervp_global.vh"
`include "ervp_axi_define.vh"
`include "hw_info.vh"
`include "sim_info.vh"

module "${MODULE_NAME}"
(
  global_rstnn,

  clk,
  rstnn,

  rhsel,
  rhready,
  rhreadyout,
  rhaddr,
  rhburst,
  rhmasterlock,
  rhprot,
  rhsize,
  rhtrans,
  rhwrite,
  rhwdata,
  rhrdata,
  rhresp,

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
localparam BW_DATA = 32;

input wire global_rstnn;
input wire clk;
input wire rstnn;

input wire rhsel;
input wire rhready;
output wire rhreadyout;
input wire [(BW_ADDR)-1:0] rhaddr;
input wire [(3)-1:0] rhburst;
input wire rhmasterlock;
input wire [(4)-1:0] rhprot;
input wire [(3)-1:0] rhsize;
input wire [(2)-1:0] rhtrans;
input wire rhwrite;
input wire [BW_DATA-1:0] rhwdata;
output wire [BW_DATA-1:0] rhrdata;
output wire rhresp;

input wire ctrl_rpsel;
input wire ctrl_rpenable;
input wire [BW_ADDR-1:0] ctrl_rpaddr;
input wire ctrl_rpwrite;
input wire [BW_DATA-1:0] ctrl_rpwdata;
output wire [BW_DATA-1:0] ctrl_rprdata;
output wire ctrl_rpready;
output wire ctrl_rpslverr;

output wire [BW_DATA-1:0] LPSDR_DQ_sod;
output wire [BW_DATA-1:0] LPSDR_DQ_soval;
input wire [BW_DATA-1:0] LPSDR_DQ_sival;
output wire [BW_DATA/8-1:0] LPSDR_DQ_sod_byte;
output wire [BW_DATA/8-1:0] LPSDR_DQ_soe_byte;

`include "sdram_cell_port_def.vh"
