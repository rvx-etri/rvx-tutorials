          endcase
        end
        direct_memory_load = 0;
      `else
        $display("[JTAG:INFO] SRAM slow load start %2d", `SRAM_HEX_SIZE);
        write_memory_using_jtag(addr, CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[0]));
        print_memory_using_jtag(addr);
        for(i=0; i<`SRAM_HEX_SIZE; i=i+1)
        begin
          if((i&32'h FF)==32'h FF)
          begin
            $display("[JTAG:INFO] SRAM slow load is processing... %8d", i);
          end
          write_memory_using_jtag(addr, CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]));
          addr = addr + 4;
        end
      `endif
      $display("[JTAG:INFO] SRAM load end");
    `endif

		// dram
    `ifdef SIMULATE_OFFCHIP_MRAM_BEHAVIOR
      $display("[JTAG:INFO] MRAM cycle %d", (OFFCHIP_MRAM_WRITE_RECOVERY_CYCLE+OFFCHIP_MRAM_WRITE_CYCLE));
      write_memory_using_jtag(`I_SYSTEM_OFFCHIP_MRAM_CONTROL_BASEADDR+`MMAP_OFFSET_MMIO_CORE_CONFIG_SAWD, (1<<(OFFCHIP_MRAM_WRITE_RECOVERY_CYCLE+OFFCHIP_MRAM_WRITE_CYCLE)));
    `endif
		`ifdef USE_LARGE_RAM
			$readmemh(`DRAM_HEX_FILE, hex_memory);
			`ifdef FAST_APP_LOAD_LARGE_RAM
        num_word_in_line = `DRAM_WIDTH/32;
				direct_memory_load = 1;
        `ifdef SIMULATE_OFFCHIP_MRAM_BEHAVIOR
  				$display("[JTAG:INFO] MRAM fast load start");
          for(i=0; i<`DRAM_HEX_SIZE; i=i+1)
          begin
            word_index = `DRAM_OFFSET + i;
            word_index_in_line = word_index % num_word_in_line;
            line_index_in_cell = word_index / num_word_in_line;
            {`MRAM_UP_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16],`MRAM_LOW_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16]} = CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]);
            //$display("%8x : %8x", word_index, hex_memory[i]);
          end
        `else
          `ifdef SIMULATE_SDRAM_BEHAVIOR
            $display("BUG due to addr mapping from sdram contorller");
            $finish;
            $display("[JTAG:INFO] SDRAM fast load start");
            num_bank_in_cell = 2;
            num_line_in_bank = `LARGE_RAM_SIZE/4/num_word_in_line/num_bank_in_cell;
            for(i=0; i<`DRAM_HEX_SIZE; i=i+1)
            begin
              word_index = `DRAM_OFFSET + i;
              word_index_in_line = word_index % num_word_in_line;
              line_index_in_cell = word_index / num_word_in_line;
              bank_index_in_cell = line_index_in_cell / num_line_in_bank;
              line_index_in_cell = line_index_in_cell % num_line_in_bank;
              if (i<10)
                $display("%8x %8x : %8x %8x %8x", word_index, CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]), bank_index_in_cell, line_index_in_cell, word_index_in_line);
              if(bank_index_in_cell==0)
                {`SDRAM_UP0_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16],`SDRAM_LOW0_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16]} = CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]);
              else
                {`SDRAM_UP1_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16],`SDRAM_LOW1_CELL[line_index_in_cell][(word_index_in_line+1)*16-1-:16]} = CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]);
              //$display("%8x : %8x", word_index, hex_memory[i]);
            end
          `else
            $display("[JTAG:INFO] Virtual RAM fast load start");
            for(i=0; i<`DRAM_HEX_SIZE; i=i+1)
            begin
              word_index = `DRAM_OFFSET + i;
              word_index_in_line = word_index % num_word_in_line;
              line_index_in_cell = word_index / num_word_in_line;
              `VLRAM_CELL[line_index_in_cell][(word_index_in_line+1)*32-1-:32] = CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]);
              //$display("%8x : %8x", word_index, hex_memory[i]);
            end
          `endif
        `endif
				direct_memory_load = 0;
			`else
        `ifdef SIMULATE_OFFCHIP_MRAM_BEHAVIOR
  				$display("[JTAG:INFO] MRAM slow load start %d", `DRAM_HEX_SIZE);
        `else
          `ifdef SIMULATE_SDRAM_BEHAVIOR
            $display("[JTAG:INFO] SDRAM slow load start %d", `DRAM_HEX_SIZE);
          `else
            $display("[JTAG:INFO] Virtual RAM slow load start %d", `DRAM_HEX_SIZE);
          `endif
        `endif
        addr = `LARGE_RAM_BASEADDR;
				for(i=0; i<`DRAM_HEX_SIZE; i=i+1)
				begin
          if((i&32'h FF)==32'h FF)
          begin
            `ifdef SIMULATE_OFFCHIP_MRAM_BEHAVIOR
              $display("[JTAG:INFO] MRAM slow load is processing... %8d", i);
            `else
              `ifdef SIMULATE_SDRAM_BEHAVIOR
                $display("[JTAG:INFO] SDRAM slow load is processing... %8d", i);
              `else
                $display("[JTAG:INFO] Virtual RAM slow load is processing... %8d", i);
              `endif
            `endif
          end
					write_memory_using_jtag(addr, CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]));
					addr = addr + 4;
				end
			`endif
      `ifdef SIMULATE_OFFCHIP_MRAM_BEHAVIOR
        $display("[JTAG:INFO] MRAM load end");
      `else
        `ifdef SIMULATE_SDRAM_BEHAVIOR
          $display("[JTAG:INFO] SDRAM load end");
        `else
          $display("[JTAG:INFO] Virtual RAM load end");
        `endif
      `endif
		`endif
		#1
		app_is_loaded = 1;
	end
end
