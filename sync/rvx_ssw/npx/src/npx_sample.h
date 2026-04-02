#ifndef __NPX_SAMPLE_H__
#define __NPX_SAMPLE_H__

#include "npx_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

// sample is the rawinput from the dataset
npx_rawinput_t *npx_load_sample(const char *filename, const char *pre_fname);

#ifdef __cplusplus
} // exter "C"
#endif

#endif // __NPX_SAMPLE_H__
