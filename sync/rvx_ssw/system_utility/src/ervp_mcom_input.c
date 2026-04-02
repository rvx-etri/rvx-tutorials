#include "platform_info.h"
#include "ervp_mcom_input.h"
#include "ervp_uart_define.h"
#include "ervp_uart.h"
#include "ervp_reg_util.h"
#include "ervp_assert.h"

#include "ervp_external_peri_group_memorymap.h"

static inline unsigned int get_base_addr(int uart_index)
{
	const unsigned int UART_BASEADDR = EXTERNAL_PERI_GROUP_BASEADDR + SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART0;
	const unsigned int addr_diff = SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART1 - SUBMODULE_ADDR_ERVP_EXTERNAL_PERI_GROUP_UART0;
	return UART_BASEADDR + uart_index*addr_diff;
}

int has_input_from_mcom(void)
{
	unsigned char lsr;
	lsr = REG8(get_base_addr(UART_INDEX_FOR_UART_PRINTF) + UART_LSR);
	return ((lsr & UART_LSR_DR) == UART_LSR_DR);
}

int getc_from_mcom(void)
{
	return uart_getc(UART_INDEX_FOR_UART_PRINTF);
}

char mcom_gui_template(const char* title, const char* key_list, const char** menu_list)
{
  assert(key_list);
  assert(menu_list);
  char key;
  int i, j;
  while(1)
  {
    i=0;
    j=0;
    printf("\n\n%s", title);
    while(1)
    {
      if(key_list[i]==0)
        break;
      if(key_list[i]==' ')
      {
        printf("\n");
        i++;
      }
      else
      {
        printf("\n%c. %s", key_list[i], menu_list[j]);
        i++;
        j++;
      }
    }
    key = getc_from_mcom();
    int is_valid_key = 0;
    i = 0;
    while(1)
    {
      if(key_list[i]==0)
        break;
      if(key==key_list[i])
      {
        is_valid_key = 1;
        break;
      }
      i++;
    }
    if(is_valid_key)
      break;
    else
      printf("\n\n > UNDEFINED INPUT (%c)", key);
  }
  return key;
}
