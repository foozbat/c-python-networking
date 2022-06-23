/*
 * COMMON FUNCTIONS
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Procedural functions accessible to all methods and drivers
 */
#pragma once

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define _XOPEN_SOURCE

#define CHUNK_SIZE 1024

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "garbagecollector.h"
#include "threadcontroller.h"
#include "devlog.h"

/// COMMON FUNCTIONS 

// init()
//   Common initialization function for server
void init();

// exit_error()
//   Gracefully exits printing error message to STDOUT
void exit_error(const char *error);

// record2array()
//   Reads in a tab delimited string and returns an array of strings with num_fields amount of elements
char **record2array(char *csv_str, int *num_fields);

// array2record()
//   Format an array of n size into tab delimited string with newline at end
char *array2record(char **arr, int num_fields);

// new_string_array()
//   Creates an empty array of strings
char **new_string_array(int num_strings);

// free_string_array()
//   Free function for arrays created by new_string_array
void free_string_array(char ***arr, int n);

// file_exists()
//   Returns true if a file exists
bool file_exists(const char *filename);

// strpos()
//   Return the position of a found string
int strpos(char *string, char *search_string);

// strip_tabs()
//   Replaces all tabs in a string with spaces
void strip_tabs(char **string);

// xor_crypt()
//   Encrypt/decrypt a string using xor cypher
void xor_crypt(char *string, const char *key);

#endif