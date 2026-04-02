#ifndef __ERVP_THREAD_H__
#define __ERVP_THREAD_H__

typedef int (*task_fx)(void *);

typedef struct
{
    int worker_core_id;
} ervp_thread_t;

enum
{
    THREAD_SUCCESS,
    THREAD_BUSY,
    THREAD_ERROR
};

// main
int thread_create(ervp_thread_t *thr, task_fx func, void *arg);
int thread_join(ervp_thread_t thr, int *res);

// worker
void _worker_thread_wait_loop();

#endif
