#ifndef __ERVP_PROFILING_H__
#define __ERVP_PROFILING_H__

#include "platform_info.h"
#include "uthash.h"

typedef struct {
  unsigned int num_called;
  int state;
  uint64_t num_tick_rshifted;
  unsigned int shift_amount;
  uint64_t previous_tick;
  char* name;
  int id;
  UT_hash_handle hh;
} profile_t;

#ifdef USE_PROFILING

void profiling_init();
profile_t* profiling_register(const char* name);
void profiling_start_by_section(profile_t* section);
void profiling_end_by_section(profile_t* section);
void profiling_start_by_name(const char* name);
void profiling_end_by_name(const char* name);
void profiling_print();

#else

static inline void profiling_init(){};
static inline profile_t* profiling_register(const char* name){ return 0; };
static inline void profiling_start_by_section(profile_t* section){};
static inline void profiling_end_by_section(profile_t* section){};
static inline void profiling_start_by_name(const char* name){};
static inline void profiling_end_by_name(const char* name){};
static inline void profiling_print(){};

#endif

#define profiling_start(name) do{ profiling_start_by_name(name); } while(0)
#define profiling_end(name) do{ profiling_end_by_name(name); } while(0)

#define PROFILING_START() \
  static profile_t *section = NULL; \
  {if (section == NULL) section = profiling_register(__func__);} \
  profiling_start_by_section(section)

#define PROFILING_END() profiling_end_by_section(section)

#endif
