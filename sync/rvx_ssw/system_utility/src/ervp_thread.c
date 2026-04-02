#include "ervp_thread.h"
#include "ervp_assert.h"
#include "ervp_variable_allocation.h"
#include "ervp_core_id.h"
#include "ervp_lock.h"
#include "ervp_mmio_util.h"

#include "ervp_core_peri_group_memorymap_offset.h"
#include "ervp_common_peri_group_memorymap.h"

#ifdef NOT_INCLUDE_CORE_WORKER

int thread_create(ervp_thread_t *thr, task_fx func, void *arg)
{
	printf_must("\n[RVX/INFO\] No Worker");
	assert(func != NULL);
	int ret = func(arg);
	thr->worker_core_id = ret;
	return THREAD_SUCCESS;
}

int thread_join(ervp_thread_t thr, int *res)
{
	if (res != NULL)
		*res = thr.worker_core_id;
	return THREAD_SUCCESS;
}

void _worker_thread_wait_loop()
{
	assert_must(0);
}

#else

/* COMMON */

typedef struct
{
	int status;
	task_fx func;
	void *arg;
	int ret;
} ervp_thread_control_block_t;

static const int TCB_STATUS_IDLE = 0;
static const int TCB_STATUS_REQUESTED = 1;
static const int TCB_STATUS_RUNNING = 2;
static const int TCB_STATUS_FINISH = 3;

static volatile ervp_thread_control_block_t tcb_buffer[NUM_CORE_WORKER] NOTCACHED_DATA;

static inline void _tcb_init(ervp_thread_control_block_t *tbc)
{
	// If this section spans multiple lines, a lock is required
	tbc->status = TCB_STATUS_IDLE;
}

static void __attribute__((constructor)) construct_ervp_thread()
{
	for (int i = 0; i < NUM_CORE_WORKER; i++)
		_tcb_init(&(tcb_buffer[i]));
}

#ifdef LOCK_INDEX_FOR_SYSTEM_VARIABLE
static const int lock_index = LOCK_INDEX_FOR_SYSTEM_VARIABLE;
#else
static const int lock_index = -1;
#endif

static inline unsigned int get_thread_status_list()
{
	return mmio_read_data(MMAP_MULTICORE_SUPPORTER_THREAD_STATUS_LIST);
}

static inline unsigned int get_thread_status(int worker_core_id)
{
	return ((get_thread_status_list() >> worker_core_id) & 1);
}

static const int WORKER_CORE_IDLE = 1;
static const int WORKER_CORE_BUSY = 0;

// MANAGER //

static inline void set_thread_status_busy(int worker_core_id)
{
	mmio_write_data(MMAP_CPG_MISC_THREAD_STATUS_LIST, (1 << worker_core_id));
}

__attribute__((weak)) int thread_create(ervp_thread_t *thr, task_fx func, void *arg)
{
	int result = THREAD_ERROR;
	volatile ervp_thread_control_block_t *tbc = NULL;

	acquire_lock_by_manager(lock_index);

	unsigned int thread_status_list = get_thread_status_list();
	if (thread_status_list == 0)
		result = THREAD_BUSY;
	else
	{
		int worker_core_id;
		for (worker_core_id = 0; worker_core_id < NUM_CORE_WORKER; worker_core_id++)
		{
			if (get_thread_status(worker_core_id) == WORKER_CORE_IDLE)
				break;
		}

		tbc = &(tcb_buffer[worker_core_id]);
		assert(tbc->status == TCB_STATUS_IDLE);
		tbc->func = func;
		tbc->arg = arg;
		tbc->status = TCB_STATUS_REQUESTED;
		set_thread_status_busy(worker_core_id);
		thr->worker_core_id = worker_core_id;
		result = THREAD_SUCCESS;
	}

	release_lock_by_manager(lock_index);

	return result;
}

__attribute__((weak)) int thread_join(ervp_thread_t thr, int *res)
{
	const int worker_core_id = thr.worker_core_id;
	assert(worker_core_id >= 0 && worker_core_id < NUM_CORE_WORKER);
	volatile ervp_thread_control_block_t *tbc = &(tcb_buffer[worker_core_id]);
	assert(tbc);

	while (1)
	{
		if (get_thread_status(worker_core_id) == WORKER_CORE_IDLE)
		{
			volatile const int status = tbc->status;
			assert(status == TCB_STATUS_FINISH);
			if (res != NULL)
				*res = tbc->ret;
			_tcb_init(tbc);
			break;
		}
	}
	return THREAD_SUCCESS;
}

// WORKER //

static inline int get_worker_core_id()
{
	return EXCLUSIVE_ID - NUM_CORE_USER;
}

static inline void set_thread_status_idle(int worker_core_id)
{
	mmio_write_data(MMAP_CPG_MISC_THREAD_STATUS_LIST, (1 << worker_core_id));
}

__attribute__((weak)) void _worker_thread_wait_loop()
{
	const int worker_core_id = get_worker_core_id();
	set_thread_status_idle(worker_core_id);
	volatile ervp_thread_control_block_t *tbc = &(tcb_buffer[worker_core_id]);

	while (1)
	{
		unsigned int thread_status_list = get_thread_status_list();
		if (get_thread_status(worker_core_id) == WORKER_CORE_BUSY)
		{
			assert(tbc->status == TCB_STATUS_REQUESTED);
			tbc->status = TCB_STATUS_RUNNING;
			assert(tbc->func != NULL);
			int ret = tbc->func(tbc->arg);
			tbc->ret = ret;
			tbc->status = TCB_STATUS_FINISH;
			set_thread_status_idle(worker_core_id);
		}
	}
}

#endif