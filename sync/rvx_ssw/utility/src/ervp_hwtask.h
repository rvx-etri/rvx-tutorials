#ifndef __ERVP_HWTASK_H__
#define __ERVP_HWTASK_H__

#include <stddef.h>
#include <stdint.h>

typedef uint64_t HWTASK_BUSY_FX_T_TYPE;
typedef HWTASK_BUSY_FX_T_TYPE ervp_hwtask_busy_fx_t;

typedef union
{
  HWTASK_BUSY_FX_T_TYPE value;
  struct
  {
    int (*busy_fx)(int);
    unsigned int task_id;
  } br;
} ervp_hwtask_busy_fx_repr_t;

static inline void hwtask_wait_complete(ervp_hwtask_busy_fx_t hwtask_busy_fx)
{
  ervp_hwtask_busy_fx_repr_t task_busy_fx2;
  task_busy_fx2.value = hwtask_busy_fx;
  if (task_busy_fx2.br.busy_fx != NULL)
  {
    while (task_busy_fx2.br.busy_fx(task_busy_fx2.br.task_id))
      ;
  }
}

#endif