/** \file
 * This file provides prototypes for an implementation of a pthread pool.
 */

#ifndef __PTHREAD_POOL_H__
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    void *arg;
    char free;
    struct pool_queue *next;
} pool_queue;

typedef struct {
    char cancelled;
    void *(*fn)(void *);
    unsigned int remaining;
    unsigned int nthreads;
    struct pool_queue *q;
    struct pool_queue *end;
    pthread_mutex_t q_mtx;
    pthread_cond_t q_cnd;
    pthread_t threads[1];
} pool;
/**
 * Create a new thread pool.
 * 
 * New tasks should be enqueued with pool_enqueue. thread_func will be called
 * once per queued task with its sole argument being the argument given to
 * pool_enqueue.
 *
 * \param thread_func The function executed by each thread for each work item.
 * \param threads The number of threads in the pool.
 * \return A pointer to the thread pool.
 */
void * pool_start(void * (*thread_func)(void *), unsigned int threads);

/**
 * Enqueue a new task for the thread pool.
 *
 * \param pool A thread pool returned by start_pool.
 * \param arg The argument to pass to the thread worker function.
 * \param free If true, the argument will be freed after the task has completed.
 */
void pool_enqueue(void *pool, void *arg, char free);

/**
 * Wait for all queued tasks to be completed.
 */
void pool_wait(void *pool);

/**
 * Stop all threads in the pool.
 *
 * Note that this function will block until all threads have terminated.
 * All queued items will also be freed, along with the pool itself.
 * Remaining work item arguments will be freed depending on the free argument to
 * pool_enqueue.
 */
void pool_end(void *pool);
#endif
