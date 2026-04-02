#include "ervp_printf.h"
#include "ervp_mmio_util.h"
#include "ervp_assert.h"

////////////////////////////////////////////////////////////////////////////////////

static inline int __is_allowed_char(char temp);

////////////////////////////////////////////////////////////////////////////////////

int __is_allowed_char(char temp)
{
	int result = 0;
	switch(temp)
	{
		case ' ':
		case '.':
		case '\\':
		case '_':
		case ':':
		case '-':
		case '/':
		case '\0':
			result = 1; break;
		default:
			if((temp>='0') && (temp<='9'))
				result = 1;
			else if((temp>='a') && (temp<='z'))
				result = 1;
			else if((temp>='A') && (temp<='Z'))
				result = 1;
			break;
	}
	return result;
}

void mmio_read_string(mmio_addr_t addr, char* dest, int max_size)
{
	int i;
	uint32_t temp;
	uint8_t temp_char;
	int invalid_string = 0;
	int start_index = 0;
	int valid = 0;

	for(i=0; i<max_size; i++)
		dest[i] = '\0';
	temp = 0;
	for(i=0; i<max_size; i++)
	{
		if((i%4)==0)
			temp = mmio_read_data(addr+(i<<1));
		switch(i%4)
		{
			case 0: temp_char = ((temp)&0xFF); break;
			case 1: temp_char = ((temp>>8)&0xFF); break;
			case 2: temp_char = ((temp>>16)&0xFF); break;
			case 3: temp_char = ((temp>>24)&0xFF); break;
		}
		//printf("character: %c %d\n", temp_char, temp_char);
		if(valid==0)
		{
			if(temp_char==' ')
				start_index++;
			else
				valid = 1;
		}
		if(valid==1)
		{
			if(temp_char=='\0')
				break;
			else if(!__is_allowed_char(temp_char))
			{
				invalid_string = 1;
				break;
			}
			else
				dest[i-start_index] = temp_char;
		}
	}
	if(invalid_string)
		printf_must("\n[RVX/MMIO/WARNING] invalid character: %c %d\n", temp_char, temp_char);
}

void mmfifo_write(mmio_addr_t addr, const void* value_list, int length)
{
	int num_vacancy = 0;
	for(int i=0; i<length; i++)
	{
		while(1)
		{
			if(num_vacancy > 0)
				break;
			num_vacancy = mmfifo_get_vacancy(addr);
			// can insert delay for intervals
		}
		//printf("\n%08x", ((unsigned int*)value_list)[i]);
		mmio_write_data(addr, ((const uint32_t*)value_list)[i]);
		num_vacancy--;
	}
}

void mmfifo_write_force(mmio_addr_t addr, const void* value_list, int length)
{
	for(int i=0; i<length; i++)
		mmio_write_data(addr, ((const uint32_t*)value_list)[i]);
}

void mmfifo_read(mmio_addr_t addr, unsigned int valid_num_addr, void* value_list, int length)
{
	int num_valid = 0;
	for(int i=0; i<length; i++)
	{
		while(1)
		{
			if(num_valid > 0)
				break;
			num_valid = mmio_read_data(valid_num_addr);
			// can insert delay for intervals
		}
		//printf("\n%08x", ((uint32_t*)value_list)[i]);
		((uint32_t*)value_list)[i] = mmio_read_data(addr);
		num_valid--;
	}
}

void mmfifo_read_force(mmio_addr_t addr, void* value_list, int length)
{
	for(int i=0; i<length; i++)
		((uint32_t*)value_list)[i] = mmio_read_data(addr);
}
