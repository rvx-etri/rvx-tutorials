#include "ervp_florian.h"
#include "ervp_florian_memorymap.h"
#include "ervp_float.h"
#include "ervp_mmio_util.h"
#include "ervp_printf.h"
#include "ervp_multicore_synch.h"

#ifdef LOCK_INDEX_FOR_SYSTEM_HW
	static const int lock_index = LOCK_INDEX_FOR_SYSTEM_HW;
#else
	static const int lock_index = -1;
#endif

static inline void wait_fifo(unsigned int addr, int num)
{
  while(1)
  {
    int num_vacancy = mmio_read_data(addr);
    if(num_vacancy>=num)
      break;
  }
}

static inline void wait_input_fifo(int num)
{
  //wait_fifo(MMAP_FLORIAN_INPUT, num);
}

static inline void wait_output_fifo(int num)
{
  //wait_fifo(MMAP_FLORIAN_OUTPUT_VALID_NUM, num);
}

static inline uint32_t CMD(unsigned int op, unsigned int input_type, unsigned int output_type)
{
  return (op<<(2*BW_FLORIAN_VALUE_TYPE)) | (input_type<<(BW_FLORIAN_VALUE_TYPE)) | (output_type);
}

#if defined(INCLUDE_FLORIAN)

float florian_convert_si32_f32 (int i)
{
  Float32Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(2);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_CONV, FLORIAN_VALUE_TYPE_SI32, FLORIAN_VALUE_TYPE_F32));
  mmio_write_data(MMAP_FLORIAN_INPUT, i);
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

float florian_convert_ui32_f32 (unsigned int i)
{
  Float32Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(2);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_CONV, FLORIAN_VALUE_TYPE_UI32, FLORIAN_VALUE_TYPE_F32));
  mmio_write_data(MMAP_FLORIAN_INPUT, i);
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

float florian_addsf3 (float a, float b)
{
  Float32Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(3);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_ADD, FLORIAN_VALUE_TYPE_F32, FLORIAN_VALUE_TYPE_F32));
  temp.value = a;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  temp.value = b;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

float florian_subsf3 (float a, float b)
{
  Float32Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(3);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_SUB, FLORIAN_VALUE_TYPE_F32, FLORIAN_VALUE_TYPE_F32));
  temp.value = a;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  temp.value = b;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

float florian_mulsf3 (float a, float b)
{
  Float32Binary temp;
  uint64_t product;
  // input
  //acquire_lock(lock_index);
  //product = ((uint64_t)calculate_float32_significand(a)) * calculate_float32_significand(b);
  wait_input_fifo(5);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_MULT, FLORIAN_VALUE_TYPE_F32, FLORIAN_VALUE_TYPE_F32));
  temp.value = a;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  temp.value = b;
  mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  product = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  product *= mmio_read_data(MMAP_FLORIAN_OUTPUT);
  mmio_write_data(MMAP_FLORIAN_INPUT, (unsigned int)product);
  mmio_write_data(MMAP_FLORIAN_INPUT, (unsigned int)(product>>32));
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

static inline uint64_t div_24_24_48(unsigned int dividend, unsigned int divisor)
{
  uint64_t first_dividend;
  uint32_t first_quotient;
  uint64_t first_remainder;
  uint64_t second_dividend;
  uint32_t second_quotient;
  uint64_t second_remainder;
  uint64_t quotient;

  first_dividend = ((uint64_t)dividend)<<15;
  first_quotient = first_dividend / divisor;
  first_remainder = first_dividend % divisor;
  second_dividend = first_remainder << 32;
  second_quotient = second_dividend / divisor;
  quotient = (((uint64_t)first_quotient)<<32) | second_quotient;
  return quotient;
}

const int FLORIAN_INCLUDE_DIVIDER = 1;

float florian_divsf3 (float a, float b)
{
  Float32Binary temp;
  uint64_t quotient;
  // input
  //acquire_lock(lock_index);
  if(FLORIAN_INCLUDE_DIVIDER)
  {
    wait_input_fifo(3);
    mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_DIV, FLORIAN_VALUE_TYPE_F32, FLORIAN_VALUE_TYPE_F32));
    temp.value = a;
    mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
    temp.value = b;
    mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
  }
  else
  {
    wait_input_fifo(5);
    mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_DIV, FLORIAN_VALUE_TYPE_F32, FLORIAN_VALUE_TYPE_F32));
    temp.value = a;
    mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
    temp.value = b;
    mmio_write_data(MMAP_FLORIAN_INPUT, temp.hex);
    quotient = div_24_24_48(mmio_read_data(MMAP_FLORIAN_OUTPUT), mmio_read_data(MMAP_FLORIAN_OUTPUT));
    mmio_write_data(MMAP_FLORIAN_INPUT, (unsigned int)quotient);
    mmio_write_data(MMAP_FLORIAN_INPUT, (unsigned int)(quotient>>32));
  }
  // output
  wait_output_fifo(1);
  temp.hex = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

#endif // defined(INCLUDE_FLORIAN)

#if defined(INCLUDE_FLORIAN_DP)

double florian_convert_si32_f64 (int i)
{
  Float64Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(2);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_CONV, FLORIAN_VALUE_TYPE_SI32, FLORIAN_VALUE_TYPE_F64));
  mmio_write_data(MMAP_FLORIAN_INPUT, i);
  // output
  wait_output_fifo(2);
  temp.int32.lower = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  temp.int32.upper = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

double florian_convert_ui32_f64 (unsigned int i)
{
  Float64Binary temp;
  // input
  //acquire_lock(lock_index);
  wait_input_fifo(2);
  mmio_write_data(MMAP_FLORIAN_INPUT, CMD(FLORIAN_OP_CONV, FLORIAN_VALUE_TYPE_UI32, FLORIAN_VALUE_TYPE_F64));
  mmio_write_data(MMAP_FLORIAN_INPUT, i);
  // output
  wait_output_fifo(2);
  temp.int32.lower = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  temp.int32.upper = mmio_read_data(MMAP_FLORIAN_OUTPUT);
  //release_lock(lock_index);
  return temp.value;
}

#endif // defined(INCLUDE_FLORIAN_DP)

#if defined(USE_FLORIAN_SP) && defined(INCLUDE_FLORIAN)

float __floatsisf (int i)
{
  return florian_convert_si32_f32(i);
}
float __floatunsisf (unsigned int i)
{
  return florian_convert_ui32_f32(i);
}

float __addsf3 (float a, float b)
{
  return florian_addsf3(a,b);
}

float __subsf3 (float a, float b)
{
  return florian_subsf3(a,b);
}

float __mulsf3 (float a, float b)
{
  return florian_mulsf3(a,b);
}

float __divsf3 (float a, float b)
{
  return florian_divsf3(a,b);
}

#endif // defined(USE_FLORIAN_SP) && defined(INCLUDE_FLORIAN)

#if defined(USE_FLORIAN_DP) && defined(INCLUDE_FLORIAN_DP)

double __floatsidf (int i)
{
  return florian_convert_si32_f64(i);
}

double __floatunsidf (unsigned int i)
{
  return florian_convert_ui32_f64(i);
}

#endif // defined(USE_FLORIAN_DP) && defined(INCLUDE_FLORIAN_DP)
