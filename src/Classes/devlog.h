/*
 * DEVLOG CLASS PROTOTYPE
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Logs desired messages to a text file
 * Usage:       Instantiate with: devlog_t *mydevlog = new_devlog()
 */
#pragma once

#ifndef DEVLOG_H_INCLUDED
#define DEVLOG_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "garbagecollector.h"

#define DEVLOG_FILENAME_LEN 256

// DEVLOG OBJECT
typedef struct
{
    int gc_id;
    FILE *fp;
    char filename[DEVLOG_FILENAME_LEN];
} devlog_t;

// EXTERNS

// METHODS
devlog_t *new_devlog(const char *);
void devlog_write(devlog_t *, const char *);
void devlog_destroy(void *);

#endif