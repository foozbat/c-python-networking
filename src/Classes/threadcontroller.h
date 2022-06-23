/*
 * THREAD CONTROLLER CLASS PROTOTYPE
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Multithreaded class to encapsulate all TCP server functionality
 * Usage:       Instantiate with: tcpserver_t *myserver = new_threadcontroller()
 */
#pragma once

#ifndef THREADCONTROLLER_H_INCLUDED
#define THREADCONTROLLER_H_INCLUDED

#define MAX_THREADS 256

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "common.h"
#include "garbagecollector.h"

// THREADCONTROLLER OBJECT
typedef struct
{
    int gc_id;
    pthread_t thread_id[MAX_THREADS];
    pthread_mutex_t tc_lock;
    pthread_mutex_t kill_lock;
} threadcontroller_t;

// THREADARGUMENTS OBJECT
typedef struct
{
    int gc_id;
    void *arg1;
    void *arg2;
    void *arg3;
    void *arg4;
    void *arg5;
} threadarguments_t;

// THREADCONTROLLER METHODS
threadcontroller_t *new_threadcontroller();
void thread_create(threadcontroller_t *, void *(*)(void *), void * );
void thread_end(threadcontroller_t *, int);
void threadcontroller_destroy(void *);

// THREADARGUMENTS METHODS
threadarguments_t *new_threadarguments();
void threadarguments_destroy(void *);

#endif