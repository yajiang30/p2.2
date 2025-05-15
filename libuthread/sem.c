#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
	size_t count;
	queue_t blocked_queue;
};

sem_t sem_create(size_t count)
{
	sem_t sem = malloc(sizeof(struct semaphore));
    if (!sem) return NULL;
    
    sem->count = count;
    sem->blocked_queue = queue_create();
    
    if (!sem->blocked_queue) {
        free(sem);
        return NULL;
    }
    
    return sem;
}

int sem_destroy(sem_t sem)
{
	if (!sem) return -1;
    
    preempt_disable();
    
    // Can't destroy if there are blocked threads
    if (queue_length(sem->blocked_queue) > 0) {
        preempt_enable();
        return -1;
    }
    
    queue_destroy(sem->blocked_queue);
    free(sem);
    
    preempt_enable();
    return 0;
}

int sem_down(sem_t sem)
{
	if (!sem) return -1;
    
    preempt_disable();
    
    if (sem->count == 0) {
        // Block current thread
        struct uthread_tcb *current = uthread_current();
        queue_enqueue(sem->blocked_queue, current);
        preempt_enable();
        uthread_block();
        // When we resume, the semaphore is ours
    } else {
        sem->count--;
        preempt_enable();
    }
    
    return 0;
}

int sem_up(sem_t sem)
{
	if (!sem) return -1;
    
    preempt_disable();
    
    sem->count++;
    
    // Unblock a waiting thread if any
    struct uthread_tcb *unblocked = NULL;
    if (queue_dequeue(sem->blocked_queue, (void **)&unblocked) == 0) {
        uthread_unblock(unblocked);
    }
    
    preempt_enable();
    return 0;
}

