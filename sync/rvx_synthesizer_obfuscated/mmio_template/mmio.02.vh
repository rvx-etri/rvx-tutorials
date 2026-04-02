/////////////
/* signals */
/////////////

genvar i;

wire [BW_DATA-1:0] man_rpwdata;
reg [BW_DATA-1:0] man_rprdata;
wire read_request_from_bus;
wire write_request_from_bus;
wire is_aligned_access;
