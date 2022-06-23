/*
 * GARBAGECOLLECTOR CLASS IMPLEMENTATION
 * Author: Aaron Bishop
 * Date:   4/16/2020
 */

#include "garbagecollector.h"

int garbage_collector_exists = 0;

// CONSTRUCTOR
garbagecollector_t *new_garbagecollector()
{
    // this object is a singleton
    if (garbage_collector_exists)
        exit_error("Attempted to instantiate second instance of singleton garbagecollector_t");

    garbagecollector_t *self = calloc(1, sizeof(garbagecollector_t));

    if (self == NULL)
    {
        exit_error("Garbagecollector memory allocation failed\n");
        return NULL;
    }

    garbage_collector_exists = 1;
    pthread_mutex_init(&(self->gc_lock), NULL);

    return self;
}

// DESTRUCTOR
void garbagecollector_cleanup(garbagecollector_t *self)
{
    for (int i=0; i<MAX_OBJECTS; i++)
        if (self->garbage_objects[i] != NULL && self->garbage_destructors[i] != NULL)
            (self->garbage_destructors[i])(self->garbage_objects[i]);

    free(self);
}

// HELPERS

/////
int _garbagecollector_get_free_index(garbagecollector_t *self)
{
    for (int i=0; i<MAX_OBJECTS; i++)
        if (self->garbage_objects[i] == NULL && self->garbage_destructors[i] == NULL)
            return i;

    return -1;
}

// METHODS

/////
int garbagecollector_register(garbagecollector_t *self, void *object, void (*destructor)(void *))
{
    pthread_mutex_lock(&(self->gc_lock));

    int next_index = _garbagecollector_get_free_index(self);

    self->garbage_objects[next_index] = object;
    self->garbage_destructors[next_index] = destructor;

    pthread_mutex_unlock(&(self->gc_lock));

    return next_index;
}

/////
void garbagecollector_unregister(garbagecollector_t *self, int gc_id)
{
    pthread_mutex_lock(&(self->gc_lock));

    self->garbage_objects[gc_id] = NULL;
    self->garbage_destructors[gc_id] = NULL;
    
    pthread_mutex_unlock(&(self->gc_lock));
}



