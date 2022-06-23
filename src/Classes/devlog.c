/*
 * DEVLOG CLASS IMPLEMENTATION
 * Author: Aaron Bishop
 * Date:   4/16/2020
 */

#include "devlog.h"

extern garbagecollector_t *global_gc;

// CONSTRUCTOR
devlog_t *new_devlog(const char *filename)
{
    devlog_t *self = calloc(1, sizeof(devlog_t));

    if (self == NULL)
        exit_error("Devlog memory allocation failed\n");

    garbagecollector_register(global_gc, (void *)self, devlog_destroy);

    strcpy(self->filename, filename);

    return self;
}

// DESTRUCTOR
void devlog_write(devlog_t *self, const char *text)
{
    self->fp = fopen(self->filename, "a");
    fprintf(self->fp, "%s\n", text);
    fclose(self->fp);
}

// METHODS

/////
void devlog_destroy(void *s) {
    devlog_t *self = (devlog_t *)s;
    garbagecollector_unregister(global_gc, self->gc_id);
    free(self);
}