#ifndef __PACT_INST_H__
#define __PACT_INST_H__

#include <stdint.h>

#include "pact_memorymap.h"
#include "pact_register_info.h"

typedef uint64_t PactInstBinary;

typedef union {
	PactInstBinary binary;
	struct
	{
		unsigned int operation;
		union Imme
		{
			int int_value;
			float float_value;
		} imme;
	} isa;
} PactImmeInst;

static inline PactInstBinary __pact_op_libnode(PactInstBinary opcode){ return ((opcode << BW_PACT_OP_PROPERTY) | PACT_OP_PROPERTY_LIBNODE | PACT_OP_PROPERTY_VALID); }
static inline PactInstBinary __pact_op_ctrl(PactInstBinary opcode){ return ((opcode << BW_PACT_OP_PROPERTY) | PACT_OP_PROPERTY_CTRL | PACT_OP_PROPERTY_VALID); }
static inline PactInstBinary __pact_op_usernode(unsigned int index){ return ((((PactInstBinary)1) << (index+BW_PACT_OP_PROPERTY)) | PACT_OP_PROPERTY_USERNODE | PACT_OP_PROPERTY_VALID); }

static inline PactInstBinary __pact_subop(PactInstBinary subopcode){ return (subopcode << (BW_PACT_NODE+BW_PACT_OP_PROPERTY)); }

static inline PactInstBinary __pact_op_lsu0fu(PactInstBinary subopcode){ return __pact_op_libnode(PACT_OP_LIBNODE_LSU0) | __pact_subop(subopcode); }
static inline PactInstBinary __pact_op_lsu1fu(PactInstBinary subopcode){ return __pact_op_libnode(PACT_OP_LIBNODE_LSU1) | __pact_subop(subopcode); }
static inline PactInstBinary __pact_op_mac0fu(PactInstBinary subopcode){ return __pact_op_libnode(PACT_OP_LIBNODE_MAC0) | __pact_subop(subopcode); }
static inline PactInstBinary __pact_op_arrange0fu(PactInstBinary subopcode){ return __pact_op_libnode(PACT_OP_LIBNODE_ARRANGE0) | __pact_subop(subopcode); }
static inline PactInstBinary __pact_op_core0fu(PactInstBinary subopcode){ return __pact_op_libnode(PACT_OP_LIBNODE_CORE0) | __pact_subop(subopcode); }

// ctrl
static inline PactInstBinary pact_inst_nop(){ return 0; }
static inline PactInstBinary pact_inst_finish(){ return __pact_op_ctrl(PACT_OP_CTRL_FINISH); }

//  fixed & float
static inline PactInstBinary pact_inst_move(unsigned int right_operand, unsigned int left_operand)
{
	return (((PactInstBinary)1)<<(left_operand+32)) | (((PactInstBinary)1)<<(right_operand+16)) | PACT_OP_PROPERTY_VALID;
}
static inline PactInstBinary pact_transpose(){ return __pact_op_arrange0fu(PACT_SUBOP_ARRANGE0_TRANS); }

static inline PactInstBinary pact_inst_lsu0_load(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_LOAD); }
static inline PactInstBinary pact_inst_lsu0_store(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_STORE); }
static inline PactInstBinary pact_inst_lsu0_info(unsigned int value)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.int_value = value;
	inst.isa.operation = __pact_op_lsu0fu(PACT_SUBOP_LSU_INFO) | PACT_OP_PROPERTY_IMME;
	return inst.binary;
}
static inline PactInstBinary pact_inst_lsu0_clean(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_CLEAN); }
static inline PactInstBinary pact_inst_lsu0_invalidate(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_INVALIDATE); }
static inline PactInstBinary pact_inst_lsu0_flush(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_FLUSH); }
static inline PactInstBinary pact_inst_lsu0_wait_cache(){ return __pact_op_lsu0fu(PACT_SUBOP_LSU_WAITCACHE); }

static inline PactInstBinary pact_inst_lsu1_load(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_LOAD); }
static inline PactInstBinary pact_inst_lsu1_store(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_STORE); }
static inline PactInstBinary pact_inst_lsu1_info(unsigned int value)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.int_value = value;
	inst.isa.operation = __pact_op_lsu1fu(PACT_SUBOP_LSU_INFO) | PACT_OP_PROPERTY_IMME;
	return inst.binary;
}
static inline PactInstBinary pact_inst_lsu1_clean(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_CLEAN); }
static inline PactInstBinary pact_inst_lsu1_invalidate(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_INVALIDATE); }
static inline PactInstBinary pact_inst_lsu1_flush(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_FLUSH); }
static inline PactInstBinary pact_inst_lsu1_wait_cache(){ return __pact_op_lsu1fu(PACT_SUBOP_LSU_WAITCACHE); }

// fixed
static inline PactInstBinary pact_inst_fixed_add(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_ADD); }
static inline PactInstBinary pact_inst_fixed_sub(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_ADD|PACT_SUBOP_MAC0_ADD_INV); }
static inline PactInstBinary pact_inst_fixed_ewmult(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_EWMULT); }
static inline PactInstBinary pact_inst_fixed_mult(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_MULT_COND|PACT_SUBOP_MAC0_INIT_ACC); }
static inline PactInstBinary pact_inst_fixed_mult_cond(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_MULT_COND); }
static inline PactInstBinary pact_inst_fixed_mac(){ return pact_inst_fixed_mult_cond(); }
static inline PactInstBinary pact_inst_fixed_scalar(int value)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.int_value = value;
	inst.isa.operation = __pact_op_arrange0fu(PACT_SUBOP_ARRANGE0_SCALAR) | PACT_OP_PROPERTY_IMME;
	return inst.binary;
}
static inline PactInstBinary pact_inst_mac0_init_acc(){ return __pact_op_mac0fu(PACT_SUBOP_MAC0_INIT_ACC); }

// float
static inline PactInstBinary pact_inst_float_add(){ return pact_inst_fixed_add() | PACT_OP_PROPERTY_FLOAT; }
static inline PactInstBinary pact_inst_float_sub(){ return pact_inst_fixed_sub() | PACT_OP_PROPERTY_FLOAT; }
static inline PactInstBinary pact_inst_float_ewmult(){ return pact_inst_fixed_ewmult() | PACT_OP_PROPERTY_FLOAT; }
static inline PactInstBinary pact_inst_float_mult(){ return pact_inst_fixed_mult() | PACT_OP_PROPERTY_FLOAT; }
static inline PactInstBinary pact_inst_float_mult_cond(){ return pact_inst_fixed_mult_cond() | PACT_OP_PROPERTY_FLOAT; }
static inline PactInstBinary pact_inst_float_mac(){ return pact_inst_float_mult_cond(); }
static inline PactInstBinary pact_inst_float_scalar(float value)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.float_value = value;
	inst.isa.operation = __pact_op_arrange0fu(PACT_SUBOP_ARRANGE0_SCALAR) | PACT_OP_PROPERTY_IMME | PACT_OP_PROPERTY_FLOAT;
	return inst.binary;
}

// core
static inline PactInstBinary pact_inst_core0_startaddr(unsigned int start_addr)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.int_value = start_addr;
	inst.isa.operation = __pact_op_core0fu(PACT_SUBOP_CORE_STARTADDR) | PACT_OP_PROPERTY_IMME;
	return inst.binary;
}

static inline PactInstBinary pact_inst_core0_active(){ return __pact_op_core0fu(PACT_SUBOP_CORE_ACTIVE); }
static inline PactInstBinary pact_inst_core0_wait(){ return __pact_op_core0fu(PACT_SUBOP_CORE_WAIT); }

// user
static inline PactInstBinary pact_inst_user_op(unsigned int op_index, unsigned int value)
{
	PactImmeInst inst;
	inst.binary = 0;
	inst.isa.imme.int_value = value;
	inst.isa.operation = __pact_op_usernode(op_index) | PACT_OP_PROPERTY_IMME;
	return inst.binary;
}

#endif
