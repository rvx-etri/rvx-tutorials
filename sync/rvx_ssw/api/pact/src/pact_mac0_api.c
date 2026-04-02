#include "ervp_malloc.h"
#include "ervp_printf.h"
#include "ervp_matrix_element.h"
#include "core_dependent.h"

#include "pact.h"
#include "pact_arch_config.h"

#ifdef PACT_INCLUDE_NODE_LSU0
static const int lsu_output_reg = PACT_LSU0_OR00;
static const int lsu_input_reg = PACT_LSU0_IR00;
static void (*fx_insert_lsu_load)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu0_load;
static void (*fx_insert_lsu_store)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu0_store;
static PactInstBinary (*fx_insert_lsu_flush)() = pact_inst_lsu0_flush;
#else
#ifdef PACT_INCLUDE_NODE_LSU1
static const int lsu_output_reg = PACT_LSU1_OR00;
static const int lsu_input_reg = PACT_LSU1_IR00;
static void (*fx_insert_lsu_load)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu1_load;
static void (*fx_insert_lsu_store)(PactFx* fx, const ErvpMatrixInfo* info) = pact_fx_insert_lsu1_store;
static PactInstBinary (*fx_insert_lsu_flush)() = pact_inst_lsu1_flush;
#endif
#endif

#ifdef PACT_INCLUDE_NODE_MAC0

 // resource conflict with multicore
static PactFx* mac_fx_load_execute = 0;
static PactFx* mac_fx_store = 0;

static PactLfx* mac_total_fx = 0;

static void __attribute__ ((constructor)) init_pact_mac0_function() 
{
	// mac_fx_load_execute
	mac_fx_load_execute = pact_fx_alloc();
	fx_insert_lsu_load(mac_fx_load_execute, &invalid_matrix_info);
	pact_fx_insert_inst(mac_fx_load_execute, pact_inst_move(lsu_output_reg, PACT_MAC0_IR00));
	fx_insert_lsu_load(mac_fx_load_execute, &invalid_matrix_info);
	pact_fx_insert_inst(mac_fx_load_execute, pact_inst_move(lsu_output_reg, PACT_MAC0_IR01));
	pact_fx_insert_reconfigure_inst(mac_fx_load_execute);
	// mac_fx_store
	mac_fx_store = pact_fx_alloc();
	pact_fx_insert_inst(mac_fx_store, pact_inst_move(PACT_MAC0_OR00, lsu_input_reg));
	fx_insert_lsu_store(mac_fx_store, &invalid_matrix_info);
	pact_fx_insert_inst(mac_fx_store, fx_insert_lsu_flush());
	// mac_total_fx
	mac_total_fx = pact_lfx_alloc();
	pact_lfx_insert_fx(mac_total_fx, mac_fx_load_execute);
	pact_lfx_insert_fx(mac_total_fx, mac_fx_store);
	//
	flush_cache();
}

static void __pact_load_and_mult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, unsigned int data_type, int acc)
{
	if(acc==1)
  {
    if(data_type == MATRIX_DATATYPE_FLOAT32)
      pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_float_mult_cond());
    else
      pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_fixed_mult_cond());
  }
  else
  {
    if(data_type == MATRIX_DATATYPE_FLOAT32)
      pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_float_mult());
    else
      pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_fixed_mult());
  }
	pact_fx_config_memory_access(mac_fx_load_execute, 0, a);
	pact_fx_config_memory_access(mac_fx_load_execute, 1, b);
	//
	pact_fx_execute(mac_fx_load_execute);
	pact_wait_until_finished();
}

static void __pact_store_mac_oreg(ErvpMatrixInfo *c)
{
	pact_fx_config_memory_access(mac_fx_store, 0, c);
	//
	pact_fx_execute(mac_fx_store);
	pact_wait_until_finished();
}

static void __pact_matrix_add(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
	if(c->datatype==MATRIX_DATATYPE_FLOAT32)
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_float_add());
	else
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_fixed_add());
	pact_fx_config_memory_access(mac_fx_load_execute, 0, a);
	pact_fx_config_memory_access(mac_fx_load_execute, 1, b);
	//
	pact_fx_config_memory_access(mac_fx_store, 0, c);
	//
	pact_lfx_execute(mac_total_fx);
	pact_wait_until_finished();
}

void pact_matrix_add(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
  int m, n;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_b = *b;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int N = a->num_col;

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_b.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;

    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_a.num_col = parted_num_rowcol;
      parted_b.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_a.addr = matrix_get_element_addr(a, m, n);
      parted_b.addr = matrix_get_element_addr(b, m, n);
      parted_c.addr = matrix_get_element_addr(c, m, n);

      __pact_matrix_add(&parted_a, &parted_b, &parted_c);
    }
  }
}

static void __pact_matrix_sub(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
	if(c->datatype==MATRIX_DATATYPE_FLOAT32)
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_float_sub());
	else
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_fixed_sub());
	pact_fx_config_memory_access(mac_fx_load_execute, 0, a);
	pact_fx_config_memory_access(mac_fx_load_execute, 1, b);
	//
	pact_fx_config_memory_access(mac_fx_store, 0, c);
	//
	pact_lfx_execute(mac_total_fx);
	pact_wait_until_finished();
}

void pact_matrix_sub(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
  int m, n;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_b = *b;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int N = a->num_col;

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_b.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;

    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_a.num_col = parted_num_rowcol;
      parted_b.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_a.addr = matrix_get_element_addr(a, m, n);
      parted_b.addr = matrix_get_element_addr(b, m, n);
      parted_c.addr = matrix_get_element_addr(c, m, n);

      __pact_matrix_sub(&parted_a, &parted_b, &parted_c);
    }
  }
}

static void __pact_matrix_ewmult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
	if(c->datatype==MATRIX_DATATYPE_FLOAT32)
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_float_ewmult());
	else
		pact_fx_set_reconfigure_inst(mac_fx_load_execute, 0, pact_inst_fixed_ewmult());
	pact_fx_config_memory_access(mac_fx_load_execute, 0, a);
	pact_fx_config_memory_access(mac_fx_load_execute, 1, b);
	//
	pact_fx_config_memory_access(mac_fx_store, 0, c);
	//
	pact_lfx_execute(mac_total_fx);
	pact_wait_until_finished();
}

void pact_matrix_ewmult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
  int m, n;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_b = *b;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int N = a->num_col;

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_b.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;

    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_a.num_col = parted_num_rowcol;
      parted_b.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_a.addr = matrix_get_element_addr(a, m, n);
      parted_b.addr = matrix_get_element_addr(b, m, n);
      parted_c.addr = matrix_get_element_addr(c, m, n);

      __pact_matrix_ewmult(&parted_a, &parted_b, &parted_c);
    }
  }
}

void pact_matrix_mult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c)
{
  int m, n, k;
  ErvpMatrixInfo parted_a = *a;
  ErvpMatrixInfo parted_b = *b;
  ErvpMatrixInfo parted_c = *c;

  const int M = a->num_row;
  const int K = a->num_col;
  const int N = b->num_col;

  unsigned int data_type = c->datatype;

  //printf("M, K, N: %d %d %d\n", M, K, N);
  if(a->num_col != b->num_row)
  {
    printf("columns number of matrix a is not equal to rows number of matrix b\n");
    return;
  }

  int parted_num_rowcol;

  for (m = 0; m < M; m += PACT_MATRIX_SIZE) {
    parted_num_rowcol = GET_PARTED_NUM_ROW_COL(M, m);
    parted_a.num_row = parted_num_rowcol;
    parted_c.num_row = parted_num_rowcol;
    for (n = 0; n < N; n += PACT_MATRIX_SIZE) {
      parted_num_rowcol = GET_PARTED_NUM_ROW_COL(N, n);
      parted_b.num_col = parted_num_rowcol;
      parted_c.num_col = parted_num_rowcol;

      parted_c.addr = matrix_get_element_addr(c, m, n);

      for (k = 0; k < K; k += PACT_MATRIX_SIZE) {
        parted_a.addr = matrix_get_element_addr(a, m, k);
        parted_b.addr = matrix_get_element_addr(b, k, n);

        parted_num_rowcol = GET_PARTED_NUM_ROW_COL(K, k);
        parted_a.num_col = parted_num_rowcol;
        parted_b.num_row = parted_num_rowcol;

        int acc = (k==0)? 0: 1;
        __pact_load_and_mult(&parted_a, &parted_b, data_type, acc);
      }
      __pact_store_mac_oreg(&parted_c);
    }
  }
}

#else // PACT_INCLUDE_NODE_MAC0

void pact_matrix_add(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c){}
void pact_matrix_sub(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c){}
void pact_matrix_ewmult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c){}
void pact_matrix_mult(const ErvpMatrixInfo *a, const ErvpMatrixInfo *b, ErvpMatrixInfo *c){}

#endif
