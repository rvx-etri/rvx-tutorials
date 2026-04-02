#ifndef __ERVP_MMIO_UTIL_H__
#define __ERVP_MMIO_UTIL_H__

#include <stdint.h>
#include "ervp_reg_util.h"
#include "ervp_round_int.h"
#include "core_dependent.h"

typedef volatile unsigned int mmio_addr_t;
typedef uint32_t mmio_data_t;
typedef void mmio_struct_t;

static inline mmio_data_t convert_endian(mmio_data_t data)
{
	mmio_data_t converted_data;
	converted_data = (data&0xFF);
	converted_data = (converted_data<<8) + ((data>>8)&0xFF);
	converted_data = (converted_data<<8) + ((data>>16)&0xFF);
	converted_data = (converted_data<<8) + ((data>>24)&0xFF);
	return converted_data;
}

static inline mmio_data_t mmio_read_data(mmio_addr_t addr)
{
	mmio_data_t data = REG32(addr);
	mmio_data_t converted_data;
	if(BIG_ENDIAN_ACCESS==1)
		converted_data = convert_endian(data);
	else
		converted_data = data;
	return converted_data;
}

static inline void mmio_read_struct(mmio_addr_t addr, int size, mmio_struct_t* data)
{
	mmio_data_t* p = (mmio_data_t*)data;
	const int size_in_4byte = rshift_ru(size,2);
	for(int i=0; i<size_in_4byte; i++)
	  p[i] = mmio_read_data(addr);
}

static inline void mmio_write_data(mmio_addr_t addr, mmio_data_t data)
{
	mmio_data_t converted_data;
	if(BIG_ENDIAN_ACCESS==1)
		converted_data = convert_endian(data);
	else
		converted_data = data;
	REG32(addr) = converted_data;
}

static inline void mmio_write_struct(mmio_addr_t addr, int size, const mmio_struct_t* data)
{
	const mmio_data_t* p = (const mmio_data_t*)data;
	const int size_in_4byte = rshift_ru(size,2);
	for(int i=0; i<size_in_4byte; i++)
	  mmio_write_data(addr, p[i]);
}

void mmio_read_string(mmio_addr_t addr, char* dest, int max_size);

static inline int mmfifo_get_vacancy(mmio_addr_t addr)
{
  return mmio_read_data(addr);
}

static inline void mmfifo_wait_unitl_ready(mmio_addr_t addr, int length)
{
  while(1)
  {
    int num_vacancy = mmfifo_get_vacancy(addr);
    if (num_vacancy>=length)
      break;
  }
}

void mmfifo_write(mmio_addr_t addr, const void* value_list, int length);
void mmfifo_write_force(mmio_addr_t addr, const void* value_list, int length);
void mmfifo_read(mmio_addr_t addr, unsigned int valid_num_addr, void* value_list, int length);
void mmfifo_read_force(mmio_addr_t addr, void* value_list, int length);

#endif
