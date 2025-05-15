#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

#define READY 0
#define BLOCKED 1
#define RUNNING 2
#define UNBLOCKED 3

struct uthread_tcb {
	uthread_ctx_t context;
	uthread_func_t func;
	void *stack;
	void *arg;
	int state;
};

queue_t threads;
queue_t hold;
struct uthread_tcb *cur_thread;

struct uthread_tcb *uthread_current(void)
{
	return cur_thread;
}

void uthread_yield(void)
{
	preempt_disable();
    
    if (queue_length(threads) == 0) {
        preempt_enable();
        return;
    }

    void *data;
    if (queue_dequeue(threads, &data) == -1) {
        preempt_enable();
        return;
    }

    struct uthread_tcb *next = (struct uthread_tcb *)data;
    queue_enqueue(threads, cur_thread);
    
    struct uthread_tcb *prev_thread = cur_thread;
    cur_thread = next;
    cur_thread->state = RUNNING;
    prev_thread->state = READY;
    
    uthread_ctx_switch(&prev_thread->context, &next->context);
    
    preempt_enable();
}

void uthread_exit(void)
{
	preempt_disable();
    
    // Free resources of exiting thread
    uthread_ctx_destroy_stack(cur_thread->stack);
    free(cur_thread);
    
    if (queue_length(threads) == 0) {
        preempt_enable();
        return;
    }

    void *data;
    if (queue_dequeue(threads, &data) == -1) {
        preempt_enable();
        return;
    }

    struct uthread_tcb *next = (struct uthread_tcb *)data;
    struct uthread_tcb *prev_thread = cur_thread;
    cur_thread = next;
    cur_thread->state = RUNNING;
    
    uthread_ctx_switch(&prev_thread->context, &next->context);
    
    // Should never reach here
    preempt_enable();
}

int uthread_create(uthread_func_t func, void *arg)
{
	preempt_disable();
    
    void* stackTop = uthread_ctx_alloc_stack();
    if (!stackTop) {
        preempt_enable();
        return -1;
    }

    struct uthread_tcb *tcb = malloc(sizeof(struct uthread_tcb));
    if (!tcb) {
        uthread_ctx_destroy_stack(stackTop);
        preempt_enable();
        return -1;
    }

    tcb->stack = stackTop;
    tcb->func = func;
    tcb->arg = arg;
    tcb->state = READY;
    
    if (uthread_ctx_init(&tcb->context, tcb->stack, tcb->func, tcb->arg) == -1) {
        uthread_ctx_destroy_stack(stackTop);
        free(tcb);
        preempt_enable();
        return -1;
    }

    if (queue_enqueue(threads, tcb) == -1) {
        uthread_ctx_destroy_stack(stackTop);
        free(tcb);
        preempt_enable();
        return -1;
    }
    
    preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	preempt_start(preempt);
    
    threads = queue_create();
    hold = queue_create();
    if (!threads || !hold) {
        if (threads) queue_destroy(threads);
        if (hold) queue_destroy(hold);
        return -1;
    }

    // Create idle thread
    struct uthread_tcb *idleTcb = malloc(sizeof(struct uthread_tcb));
    if (!idleTcb) {
        queue_destroy(threads);
        queue_destroy(hold);
        return -1;
    }
    idleTcb->state = RUNNING;
    cur_thread = idleTcb;

    if (uthread_create(func, arg) == -1) {
        free(idleTcb);
        queue_destroy(threads);
        queue_destroy(hold);
        return -1;
    }

    while (queue_length(threads) > 0) {
        uthread_yield();
    }

    // Cleanup
    free(idleTcb);
    queue_destroy(threads);
    queue_destroy(hold);
    preempt_stop();
    
    return 0;
}

void uthread_block(void)
{
	preempt_disable();
    cur_thread->state = BLOCKED;
    queue_enqueue(hold, cur_thread);
    uthread_yield();
    preempt_enable();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	preempt_disable();
    void *tempData;
    if (queue_dequeue(hold, &tempData) == 0) {
        uthread->state = READY;
        queue_enqueue(threads, uthread);
    }
    preempt_enable();
}

