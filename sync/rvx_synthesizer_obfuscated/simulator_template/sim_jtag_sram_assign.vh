            "${CELL_INDEX}":
            begin
              `SRAM_CELL"${CELL_INDEX}"_VARIABLE[line_index_in_cell][(word_index_in_line+1)*`BW_WORD-1-:`BW_WORD] = CHANGE_ENDIAN_HEX2MAN(32,`MEMORY_ENDIAN,hex_memory[i]);
            end
