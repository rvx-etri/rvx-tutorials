///////////
/* ncsim */
///////////

assign ncsim_finish_force = 0;

`ifdef INCLUDE_UART_PRINTF
	assign ncsim_uart_rx = printf_tx;
	assign printf_rx = ncsim_uart_tx;
`else
	assign ncsim_uart_rx = 0;
`endif

NCSIM_MANAGER
#(
	.UART_CLK_HZ(`UART_CLK_HZ)
)
i_ncsim_manager
(
  .finish_force(ncsim_finish_force),
  .uart_rx(ncsim_uart_rx),
	.uart_tx(ncsim_uart_tx),
  .record_enable(record_enable),
  .dump_enable(dump_enable),
  .simulation_stop(simulation_stop)
);

`include "ncsim_record.vb"
`include "ncsim_dump.vb"
