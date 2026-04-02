);

////////////////////////////
/* parameter input output */
////////////////////////////

parameter BW_ADDR = 1;
parameter BW_DATA = 1;
parameter ENDIAN_TYPE = `LITTLE_ENDIAN;

`include "ervp_endian.vf"
`include "ervp_log_util.vf"

input wire clk, rstnn;
input wire rpsel;
input wire rpenable;
input wire [BW_ADDR-1:0] rpaddr;
input wire rpwrite;
input wire [BW_DATA-1:0] rpwdata;
output wire [BW_DATA-1:0] rprdata;
output reg rpready;
output reg rpslverr;
