#include "ervp_fakefile.h"
#include "ervp_malloc.h"
#include "ervp_printf.h"

#include <string.h>
#include <limits.h>

FAKEFILE *ffopen_(char *pt, int mem_size)
{
  FAKEFILE *mem = malloc(sizeof(FAKEFILE));

  mem->base_pt = pt;
  mem->current_pt = pt;
  mem->end_pt = pt + mem_size;

  return mem;
}       

char *ffgetl_(FAKEFILE *mem)
{
  char *line;    
  if(mem->current_pt < mem->end_pt)
  {
    line = (char *)mem->current_pt;
    while(1)
    {
      if(*(char *)mem->current_pt == '\0')
      { 
        mem->current_pt++;
        break;
      }
      else
        mem->current_pt++;
    }
  }
  else
  {       
    line = 0;
  }

  return line;
}

void ffread_(void *dest, int size, int count, FAKEFILE *mem)
{
  int i;
  int tot_size = size * count;
  for(i = 0; i < tot_size; i++)
  {
    ((char *)dest)[i] = *(char *)mem->current_pt++;
  }
}
