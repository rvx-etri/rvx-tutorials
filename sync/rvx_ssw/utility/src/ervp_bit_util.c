#include "ervp_bit_util.h"
#include "ervp_assert.h"

int get_max_value(int bitwidth)
{
	return (1 << (bitwidth - 1)) - 1;
}

unsigned int convert_sign_and_magnitude(int value, int bitwidth)
{
	int max_signed_value, min_signed_value;
	max_signed_value = get_max_value(bitwidth);
	min_signed_value = -max_signed_value;
	assert(value <= max_signed_value);
	assert(value >= min_signed_value);

	unsigned int result;
	int is_minus;
	int max_magnitude = generate_mask_by_size(bitwidth - 1);
	is_minus = (value < 0);
	result = is_minus ? (-value) : value;
	if (is_minus)
		result |= (-1 - max_magnitude);
	return result;
}