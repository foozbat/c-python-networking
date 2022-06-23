/*
 * GARBAGECOLLECTOR CLASS PROTOTYPE
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Maintains a registry of created object and destructors, and frees all memory when called.
 * Usage:       Instantiate with: garbagecollector_t *mygc = new_garbagecollector()
 */
#pragma once

#ifndef GARBAGECOLLECTOR_H_INCLUDED
#define GARBAGECOLLECTOR_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define MAX_OBJECTS 5120

// GARBAGE COLLECTOR OBJECT
typedef struct
{
    void *garbage_objects[MAX_OBJECTS];
    void (*garbage_destructors[MAX_OBJECTS])(void *);
    pthread_mutex_t gc_lock;

} garbagecollector_t;

// CONSTRUCTOR
garbagecollector_t *new_garbagecollector();

// "DESTRUCTOR" (called directly by sig handler)
void garbagecollector_cleanup(garbagecollector_t *);

// METHODS

// garbagecollector_register()
//   Registers an object's destructor with the GC
//   This method should be called in an objects constructor
int garbagecollector_register(garbagecollector_t *, void *, void (*)(void *));

// garbagecollector_unregister()
//   Removes an object's destructor from the GC
//   This function should be called from an object's destructor
void garbagecollector_unregister(garbagecollector_t *, int);

#endif