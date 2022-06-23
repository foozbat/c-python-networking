/*
    CATALOG CLASS PROTOTYPE
    Author:      Aaron Bishop
    Date:        4/17/2020
    Description: Catalog of all books in the library
    Usage:       Instantiate with: catalog_t *mycatalog = new_catalog()

    CATALOG DATABASE SCHEMA
    id,date_created,date_updated,book_name,qty_total
    

*/
#pragma once

#ifndef CATALOG_H_INCLUDED
#define CATALOG_H_INCLUDED

#include <stdbool.h>

#include "garbagecollector.h"
#include "datafile.h"

#define CATALOG_DB_FILENAME "data/catalog.db"
#define CATALOG_REQUESTS_DB_FILENAME "data/catalog_requests.db"
#define CATALOG_BOOK_NAME_LEN 13
#define CATALOG_AVAIL_LINE_LEN 62

// CATALOG OBJECT
typedef struct
{
    int gc_id;

    datafile_t *catalog_db;
    datafile_t *requests_db;

} catalog_t;

// CONSTRUCTOR
catalog_t *new_catalog();

// DESTRUCTOR
void catalog_destroy(void *);

// METHODS

// catalog_add_book()
//   Adds a book to the catalog with specified quantity
//   If book already exists, adds quantity to total
bool catalog_add_book(catalog_t *self, const char *book_name, int qty);

// catalog_get_book_avail_qty()
//   returns "available" quantity of specified book
//   Available qty is total qty minus number of books requested
int catalog_get_book_avail_qty(catalog_t *self, const char *book_name);

// catalog_request_book()
//   Requests qty books if specified book exists
//   Stores book request on a per-user, per-book basis
bool catalog_request_book(catalog_t *self, const char *book_name, int user_id, int qty_requested);

// catalog_return_book()
//   Returns qty books if specified book exists
bool catalog_return_book(catalog_t *self, const char *book_name, int user_id, int qty_returning);

// catalog_get_book_id()
//   Returns the unique id of a specified book, 0 if not found
int catalog_get_book_id(catalog_t *self, const char *book_name);

// catalog_book_exists()
//   Returns true if a book exists
bool catalog_book_exists(catalog_t *self, const char *book_name);

// catalog_generate_report()
//   Generates an inventory report and saves it to the reports/ subfolder
//   Report format: BOOK NAME          TOTAL ON HAND            IN USE         AVAILABLE
//   Report filename format: inventory_report_<userid>_<date>_<time>.txt
bool catalog_generate_report(catalog_t *self, const char *report_filename);

// catalog_get_availability_report()
//   Generates an availability report and returns it as a string
//   Report format: BOOK NAME          TOTAL ON HAND        AVAILABLE
char *catalog_get_availability_report(catalog_t *self);

#endif