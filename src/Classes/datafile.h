/*
    DATAFILE CLASS PROTOTYPE
    Author:      Aaron Bishop
    Date:        4/17/2020
    Description: Abstraction for working with csv text data files
    Usage:       Instantiate with: datafile_t *mydatafile = new_datafile()
                 
                 Datafile format:
                 id,created,updated,field1,field2,field3

 */

 #pragma once

#ifndef DATAFILE_H_INCLUDED
#define DATAFILE_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/file.h>

#include "garbagecollector.h"

#define DATAFILE_FILENAME_MAXLEN 256
#define DATAFILE_ROW_MAXLEN 4096

typedef struct
{
    int gc_id;

    char filename[DATAFILE_FILENAME_MAXLEN];

    int num_fields;         // number of fields in the data file
    int num_data_fields;    // number of data fields in the data file (excludes the first 3 auto fields)
    char **field_names;     // name of the fields read from header of data file
    int last_row_id;       // stores last id returned for get_row_by_field
    int header_len;

} datafile_t;

// CONSTRUCTOR
datafile_t *new_datafile(const char *filename);

// DESTRUCTOR
void datafile_destroy(void *);

// methods to manipulate datafile rows
bool datafile_add_row(datafile_t *self, char ***row_data);
bool datafile_update_row(datafile_t *self, int id, char ***row_data);
bool datafile_delete_row(datafile_t *self, int id);

// methods to retrieve datafile rows
void datafile_get_row_prepare(datafile_t *self);
char **datafile_get_row(datafile_t *self);
char **datafile_get_row_by_field(datafile_t *self, const char *field_name, const char *field_value);

// methods to manipulate row arrays
char **datafile_new_row_array(datafile_t *self);
bool datafile_set_col(datafile_t *self, char ***row_data, const char *field_name, const char *col_data);
int datafile_get_field_index(datafile_t *self, const char *field_name);
void datafile_free_row(datafile_t *self, char ***row_data);

// copy a file from src to dst
bool _datafile_copy_file_data(const char *source, const char *dest, unsigned long offset, unsigned long sz);

#endif