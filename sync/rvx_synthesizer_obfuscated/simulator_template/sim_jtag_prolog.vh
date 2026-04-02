//////////
/* jtag */
//////////

assign boot_mode = `BOOT_MODE_OCD;

`include "ervp_endian.vf"
`include "ervp_jtag_util.vf"
`include "ervp_platform_controller_util.vf"

`ifndef SRAM_HEX_FILE
`define SRAM_HEX_FILE "sram.hex"
`endif

`ifndef SRAM_OFFSET
`define SRAM_OFFSET 32'h 0
`endif

`ifndef CRM_HEX_FILE
`define CRM_HEX_FILE "crm.hex"
`endif

`ifndef CRM_OFFSET
`define CRM_OFFSET 32'h 0
`endif

`ifndef DRAM_HEX_FILE
`define DRAM_HEX_FILE "dram.hex"
`endif

`ifndef DRAM_OFFSET
`define DRAM_OFFSET 32'h 0
`endif

`ifndef MEMORY_ENDIAN
`define MEMORY_ENDIAN `LITTLE_ENDIAN
`endif

`ifdef FAST_APP_LOAD
`ifdef USE_LARGE_RAM
`ifndef SIMULATE_SDRAM_BEHAVIOR
`define FAST_APP_LOAD_LARGE_RAM
`endif
`endif // USE_LARGE_RAM
`endif

`ifdef FAST_APP_LOAD_LARGE_RAM
`define VLRAM_CELL i_platform.`DRAM_IP_INSTANCE.i_cell.memory
`define MRAM_UP_CELL i_mram_up.MRAM_Array
`define MRAM_LOW_CELL i_mram_low.MRAM_Array
`endif

`define BW_WORD 32
`define SRAM_CELL_SIZE_IN_WORD (`SRAM_CELL_SIZE/(`BW_WORD/`BW_BYTE))

integer reset_cycle;
reg first_reset_is_complete;

reg app_load_request;
reg app_is_loaded;

integer i,j;
integer word_index;
integer cell_index;
integer word_index_in_cell;
integer word_index_in_line;
integer line_index_in_cell;
integer direct_memory_load;
integer addr;
integer num_word_in_line;
integer ddr_file;
integer error;

reg [32-1:0] hex_memory [`HEX_SIZE-1:0];

initial
begin
	pjtag_rfinished = 0;
	app_load_request = 0;
	#1
	wait(rstnn==0);
	wait(rstnn==1);
	if(boot_mode==`BOOT_MODE_STAND_ALONE)
	begin
		$display("[JTAG:INFO] do NOTHING");
	end
	else if(boot_mode==`BOOT_MODE_OCD)
	begin
		wait(first_reset_is_complete==1);
		init_jtag();
		release_all_reset();
		app_load_request = 1;
		wait(app_is_loaded==1);
    set_app_addr();
		set_sim_env();
		set_all_ready();
	end
	else
	begin
		$display("[JTAG:ERROR] undefined boot mode");
		$finish;
	end
	#1
	$display("[JTAG:INFO] finished");
	pjtag_rfinished = 1;
end

initial
begin
	first_reset_is_complete = 0;
	reset_cycle = 0;
	#1
	wait(rstnn==0);
	wait(rstnn==1);
	while(1)
	begin
		wait(clk==0);
		wait(clk==1);
		if(reset_cycle==100*20)
			break;
		else
			reset_cycle = reset_cycle + 1;
	end
	first_reset_is_complete = 1;
end

initial
begin
  `ifdef USE_SMALL_RAM
    if((`SRAM_HEX_SIZE*4) > `SIM_SMALL_RAM_SIZE)
    begin
      $display("[Error] SIM_SMALL_RAM_SIZE is too small");
      $finish;
    end
  `endif
  `ifdef USE_LARGE_RAM
    if((`DRAM_HEX_SIZE*4) > `SIM_LARGE_RAM_SIZE)
    begin
      $display("[Error] SIM_LARGE_RAM_SIZE is too small");
      $finish;
    end
  `endif
  `ifndef FAST_APP_LOAD
    $display("[Warning] Please define BUILD_MODE=RELEASE");
    $display("[Warning] in rvx_each.mh or ../user/sim/env/set_sim_env.mh");
    $display("[Warning] Or simulation takes too long\n");
  `endif
end

initial
begin
	app_is_loaded = 0;
	direct_memory_load = 0;
	#1
	wait(rstnn==0);
	wait(rstnn==1);
	if(boot_mode==`BOOT_MODE_OCD)
	begin
		wait(app_load_request==1);
		#1
		// sram
		`ifdef USE_SMALL_RAM
      $readmemh(`SRAM_HEX_FILE, hex_memory);
      addr = `SMALL_RAM_BASEADDR;
      `ifdef FAST_APP_LOAD
        direct_memory_load = 1;
        num_word_in_line = `SRAM_CELL_VARIABLE_WIDTH/`BW_WORD;
        $display("[JTAG:INFO] SRAM fast load start");
        for(i=0; i<`SRAM_HEX_SIZE; i=i+1)
        begin
          word_index = `SRAM_OFFSET + i;
          cell_index = word_index / `SRAM_CELL_SIZE_IN_WORD;
          word_index_in_cell = word_index % `SRAM_CELL_SIZE_IN_WORD;
          line_index_in_cell = word_index_in_cell / num_word_in_line;
          word_index_in_line = word_index_in_cell % num_word_in_line;
