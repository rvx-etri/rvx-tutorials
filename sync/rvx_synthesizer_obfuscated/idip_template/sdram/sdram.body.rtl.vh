ERVP_SDRAM_CONTROLLER
#(
  .BW_ADDR(BW_ADDR)
)
i_controller
(
  .global_rstnn(global_rstnn),
  .clk(clk),
  .rstnn(rstnn),

  .rhsel(rhsel),
  .rhready(rhready),
  .rhreadyout(rhreadyout),
  .rhaddr(rhaddr),
  .rhburst(rhburst),
  .rhmasterlock(rhmasterlock),
  .rhprot(rhprot),
  .rhsize(rhsize),
  .rhtrans(rhtrans),
  .rhwrite(rhwrite),
  .rhwdata(rhwdata),
  .rhrdata(rhrdata),
  .rhresp(rhresp),

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
