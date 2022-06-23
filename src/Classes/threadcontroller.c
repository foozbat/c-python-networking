/*
 * THREAD CONTROLLER CLASS IMPLEMENTATION
 * Author: Aaron Bishop
 * Date:   4/16/2020
 */

#include "threadcontroller.h"

int thread_controller_exists = 0;
extern garbagecollector_t *global_gc;

// CONSTRUCTOR
threadcontroller_t *new_threadcontroller()
{
    // this object is a singleton
    if (thread_controller_exists)
        exit_error("Attempted to instantiate second instance of singleton threadcontroller_t");

    threadcontroller_t *self = calloc(1, sizeof(threadcontroller_t));

    if (self == NULL)
        exit_error("Thread controller memory allocation failed\n");

    // regiser with garbage collection
    garbagecollector_register(global_gc, (void *)self, threadcontroller_destroy);

    // setup mutexes
    pthread_mutex_init(&(self->tc_lock), NULL);
    pthread_mutex_init(&(self->kill_lock), NULL);
    pthread_mutex_lock(&(self->kill_lock));

    thread_controller_exists = 1;

    return self;
}

// DESTRUCTOR
void threadcontroller_destroy(void *s)
{
    threadcontroller_t *self = (threadcontroller_t *)s;

    // gracefully kill threads that utilize the kill lock
    pthread_mutex_unlock(&(self->kill_lock));

    // kill all remaining threads
    for (int i=0; i<MAX_THREADS; i++)
        if (self->thread_id[i] != 0)
            thread_end(self, i);

    garbagecollector_unregister(global_gc, self->gc_id);

    free(self);
}

// HELPERS

int _threadcontroller_get_free_index(threadcontroller_t *self)
{
    for (int i=0; i<MAX_THREADS; i++)
        if (self->thread_id[i] == 0)
            return i;

    return -1;
}

// METHODS

/////
void thread_create(threadcontroller_t *self, void *(*start_routine)(void *), void *arg)
{
    threadarguments_t *thread_arguments = (threadarguments_t *)arg;
    
    pthread_mutex_lock(&(self->tc_lock));
    int thread_index = _threadcontroller_get_free_index(self);

    // set the first argument to the thread_id
    thread_arguments->arg1 = &self->thread_id[thread_index];
    
    if (thread_index >= 0)
    {
        int rc = pthread_create(&self->thread_id[thread_index], NULL, start_routine, thread_arguments);

        if (rc)
            exit_error("Could not create new thread");
    }
    pthread_mutex_unlock(&(self->tc_lock));
}

/////
void thread_end(threadcontroller_t *self, int thread_index)
{
    pthread_join(self->thread_id[thread_index], NULL);

    pthread_mutex_lock(&(self->tc_lock));
    self->thread_id[thread_index] = 0;
    pthread_mutex_unlock(&(self->tc_lock));
}

// THREADARGUMENTS METHODS //

// CONSTRUCTOR
threadarguments_t *new_threadarguments()
{
    threadarguments_t *self = calloc(1, sizeof(threadarguments_t));

    if (self == NULL)
        exit_error("Thread arguments memory allocation failed");

    garbagecollector_register(global_gc, (void *)self, threadarguments_destroy);

    return self;
}

// DESTRUCTOR
void threadarguments_destroy(void *s)
{
    threadarguments_t *self = (threadarguments_t *)s;

    garbagecollector_unregister(global_gc, self->gc_id);

    free(self);
}
