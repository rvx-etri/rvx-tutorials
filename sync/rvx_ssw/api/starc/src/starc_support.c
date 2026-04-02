#include "uthash.h"

#include "starc_support.h"
#include "ervp_malloc.h"
#include "ervp_bit_util.h"
#include "ervp_assert.h"
#include "ervp_matrix.h"
#include "ervp_matrix_element.h"

/*
typedef struct convert_info
{
	const ErvpMatrixInfo *original;
	const ErvpMatrixInfo *converted;
	UT_hash_handle hh;
} convert_info_t;

typedef struct ip_convert_info
{
	const starc_hwinfo_t *hwinfo;
	convert_info_t *convert_dict;
	UT_hash_handle hh;
} ip_convert_info_t;

static ip_convert_info_t *ip_convert_dict = NULL;

static const ErvpMatrixInfo *convert_weight(const ErvpMatrixInfo *weight_info, int bw_weight)
{
	assert(!matrix_datatype_is_float(weight_info->datatype));
	ErvpMatrixInfo *converted = matrix_alloc(weight_info->datatype, weight_info->num_row, weight_info->num_col, NULL);
	int max_magnitude = generate_mask_by_size(bw_weight - 1);
	for (int i = 0; i < weight_info->num_row; i++)
	{
		for (int j = 0; j < weight_info->num_col; j++)
		{
			UNKNOWN_TYPE data = _matrix_read_element(weight_info, i, j);
			data.value_unsigned = convert_sign_and_magnitude(data.value_signed, bw_weight);
			_matrix_write_element(converted, i, j, data);
		}
	}
	return converted;
}

const ErvpMatrixInfo *starc_convert_weight(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info)
{
	ip_convert_info_t *ip_convert_info;
	convert_info_t *convert_info;
	const ErvpMatrixInfo *result;
	HASH_FIND_INT(ip_convert_dict, &hwinfo, ip_convert_info);
	if (ip_convert_info == NULL)
	{
		ip_convert_info = malloc(sizeof(ip_convert_info_t));
		assert(ip_convert_info);
		ip_convert_info->hwinfo = hwinfo;
		ip_convert_info->convert_dict = NULL;
		HASH_ADD_INT(ip_convert_dict, hwinfo, ip_convert_info);
	}
	HASH_FIND_INT(ip_convert_info->convert_dict, &weight_info, convert_info);
	// printf("\n\n%p", ip_convert_info);
	// printf("\n%p", convert_info);
	if (convert_info == NULL)
	{
		convert_info = malloc(sizeof(convert_info_t));
		assert(convert_info);
		convert_info->original = weight_info;
		convert_info->converted = convert_weight(weight_info, hwinfo->bw_weight);
		HASH_ADD_INT(ip_convert_info->convert_dict, original, convert_info);
		result = convert_info->converted;

		convert_info = malloc(sizeof(convert_info_t));
		assert(convert_info);
		convert_info->original = result;
		convert_info->converted = NULL;
		HASH_ADD_INT(ip_convert_info->convert_dict, original, convert_info);
	}
	else
	{
		if (convert_info->converted == NULL)
			result = weight_info;
		else
			result = convert_info->converted;
	}
	return result;
}
*/

const ErvpMatrixInfo *starc_convert_weight(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info)
{
	assert(!matrix_datatype_is_float(weight_info->datatype));
	int bw_weight = hwinfo->bw_weight;
	ErvpMatrixInfo *converted = matrix_alloc(weight_info->datatype, weight_info->num_row, weight_info->num_col, NULL);
	assert(converted);
	for (int i = 0; i < weight_info->num_row; i++)
	{
		for (int j = 0; j < weight_info->num_col; j++)
		{
			UNKNOWN_TYPE data = _matrix_read_element(weight_info, i, j);
			data.value_unsigned = convert_sign_and_magnitude(data.value_signed, bw_weight);
			_matrix_write_element(converted, i, j, data);
		}
	}
	return converted;
}

unsigned int starc_analyze_num_spike_allowed(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info, const ervp_starc_option_t *options)
{
	assert(options != NULL);
	unsigned int result;
	result = (hwinfo->core_input_size + 1) >> 1;
	return result;
}

static inline unsigned int starc_get_weight_max(const starc_hwinfo_t *hwinfo)
{
	return math_exp_uint(2, hwinfo->bw_weight - 1) - 1;
}

uint32_t *starc_convert_threshold(const starc_hwinfo_t *hwinfo, const ervp_starc_option_t *options)
{
	assert(options != NULL);
	assert(options->threshold != 0);

	uint32_t *formatted_threshold = malloc(sizeof(uint32_t *) * hwinfo->core_input_size);
	assert(formatted_threshold);
	const unsigned int weight_max = starc_get_weight_max(hwinfo);
	int remaining_threshold = options->threshold;

	for (int i = 0; i < hwinfo->core_input_size; i++)
	{
		int value;
		if (remaining_threshold == 0)
			formatted_threshold[i] = 0;
		else
		{
			if (remaining_threshold >= weight_max)
				value = weight_max;
			else
				value = remaining_threshold;
			formatted_threshold[i] = convert_sign_and_magnitude(-value, hwinfo->bw_weight);
			remaining_threshold -= value;
		}
	}
	assert(remaining_threshold == 0);
	return formatted_threshold;
}

starc_supply_t *starc_supply_generate(const starc_hwinfo_t *hwinfo, const ErvpMatrixInfo *weight_info, const ervp_starc_option_t *options)
{
	starc_supply_t *result = malloc(sizeof(starc_supply_t));
	result->converted_weight_info = starc_convert_weight(hwinfo, weight_info);
	result->num_spike_allowed = starc_analyze_num_spike_allowed(hwinfo, weight_info, options);
	result->formatted_threshold = starc_convert_threshold(hwinfo, options);
	return result;
}

void starc_supply_free(starc_supply_t *supply)
{
	matrix_free(supply->converted_weight_info);
	free(supply->formatted_threshold);
	free(supply);
}