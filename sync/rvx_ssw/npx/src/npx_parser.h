#ifndef __NPX_PARSER_H__
#define __NPX_PARSER_H__

#include "ervp_fakefile.h"
#include "texpar_list.h"
#include "texpar_api.h"
#include "npx_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif

  npx_network_t *npx_parse_network_cfg(const char *net_fname, const char *opt_fname);

#ifdef __cplusplus
} // exter "C"
#endif

#endif // __NPX_PARSER_H__
