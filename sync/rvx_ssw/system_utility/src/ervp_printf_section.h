#ifndef __ERVP_PRINTF_SECTION_H__
#define __ERVP_PRINTF_SECTION_H__

#include "ervp_platform_api.h"
#include "ervp_platform_controller_api.h"
#include "ervp_printf.h"

#define printf_section(skip_sim, ...) { if(!(skip_sim && is_sim())) {printf("\n\n***[[ "); printf(__VA_ARGS__); printf(" ]]***\n");} }
#define printf_subsection(skip_sim, ...) { if(!(skip_sim && is_sim())) {printf("\n\n**[ "); printf(__VA_ARGS__); printf(" ]**\n");} }
#define printf_subsubsection(skip_sim, ...) { if(!(skip_sim && is_sim())) {printf("\n\n*[ "); printf(__VA_ARGS__); printf(" ]*");} }

#endif  // __ERVP_PRINTF_SECTION_H__
