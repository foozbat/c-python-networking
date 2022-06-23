/*
    CATALOG CLASS IMPLEMENTATION
    Author: Aaron Bishop
    Date:   4/17/2020
*/

#include "catalog.h"

extern garbagecollector_t *global_gc;

////
catalog_t *new_catalog()
{
    catalog_t *self = calloc(1, sizeof(catalog_t));

    if (self == NULL)
        exit_error("Catalog memory allocation failed\n");

    garbagecollector_register(global_gc, (void *)self, catalog_destroy);

    self->catalog_db = new_datafile(CATALOG_DB_FILENAME);
    self->requests_db = new_datafile(CATALOG_REQUESTS_DB_FILENAME);

    if (self->catalog_db == NULL)
        exit_error("Failed to initialize catalog database");
    if (self->requests_db == NULL)
        exit_error("Failed to initialize requests database");

    return self;
}

////
void catalog_destroy(void *s)
{
    catalog_t *self = (catalog_t *)s;
    garbagecollector_unregister(global_gc, self->gc_id);
    free(self);
}

////
bool catalog_add_book(catalog_t *self, const char *book_name, int qty)
{
    if (self == NULL || book_name == NULL)
        return false;

    if (strcmp(book_name, "") == 0)
        return false;

    int book_id = catalog_get_book_id(self, book_name);

    char qty_str[10];
    char **book_data = datafile_new_row_array(self->catalog_db);
    datafile_set_col(self->catalog_db, &book_data, "book_name", book_name);

    if (!book_id)
    {
        // create new catalog entry
        sprintf(qty_str, "%d", qty);
        datafile_set_col(self->catalog_db, &book_data, "qty_total", qty_str);
        datafile_add_row(self->catalog_db, &book_data);
        
    }
    else
    {
        // update existing entry with total_qty += qty
        datafile_get_row_prepare(self->catalog_db);
        char **row = datafile_get_row_by_field(self->catalog_db, "book_name", book_name);

        if (row == NULL)
            return 0;

        int total_qty = atoi(row[datafile_get_field_index(self->catalog_db, "qty_total")]);
        datafile_free_row(self->catalog_db, &row);

        sprintf(qty_str, "%d", total_qty + qty);
        datafile_set_col(self->catalog_db, &book_data, "qty_total", qty_str);
        datafile_update_row(self->catalog_db, book_id, &book_data);
    }
    datafile_free_row(self->catalog_db, &book_data);

    return true;
}

/////
int catalog_get_book_avail_qty(catalog_t *self, const char *book_name)
{
    if (self == NULL || book_name == NULL)
        return 0;

    int ret_qty = 0;

    // get the total qty for this book
    datafile_get_row_prepare(self->catalog_db);
    char **row_book = datafile_get_row_by_field(self->catalog_db, "book_name", book_name);

    if (row_book == NULL)
        return 0;

    ret_qty = atoi(row_book[datafile_get_field_index(self->catalog_db, "qty_total")]);

    char **row_requests = NULL;

    datafile_get_row_prepare(self->requests_db);

    // subtract qty for each user's book request from total qty
    while ((row_requests = datafile_get_row_by_field(self->requests_db, "book_id", row_book[0])) != NULL)
    {
       ret_qty -= atoi(row_requests[datafile_get_field_index(self->requests_db, "qty_requested")]);
       datafile_free_row(self->requests_db, &row_requests);
    }

    datafile_free_row(self->catalog_db, &row_book);

    return ret_qty;
}

////
bool catalog_request_book(catalog_t *self, const char *book_name, int user_id, int qty_requested)
{
    if (self == NULL || book_name == NULL || user_id < 1)
        return false;

    int book_id = catalog_get_book_id(self, book_name);

    if (!book_id)
        return false;
        
    int qty_avail = catalog_get_book_avail_qty(self, book_name);

    if (qty_requested > qty_avail)
        return false;

    char user_id_str[10];
    char book_id_str[10];
    char qty_requested_str[10];
    sprintf(user_id_str, "%d", user_id);
    sprintf(book_id_str, "%d", book_id);

    // see if this user has requests for this book
    int qty_already_requested = 0;
    int request_id = 0;
    char **row_requests = NULL;
    datafile_get_row_prepare(self->requests_db);

    while ((row_requests = datafile_get_row_by_field(self->requests_db, "user_id", user_id_str)) != NULL)
    {
        if (strcmp(row_requests[datafile_get_field_index(self->requests_db, "book_id")], book_id_str) == 0)
        {
            request_id = atoi(row_requests[0]);
            qty_already_requested = atoi(row_requests[datafile_get_field_index(self->requests_db, "qty_requested")]);
            datafile_free_row(self->requests_db, &row_requests);
            break;
        }    
        datafile_free_row(self->requests_db, &row_requests);
    }

    qty_requested += qty_already_requested;

    if (qty_requested < 0)
        return false;

    if (qty_requested <= qty_avail)
    {
        // prepare book request row
        char **request_book_data = datafile_new_row_array(self->requests_db);
        datafile_set_col(self->requests_db, &request_book_data, "user_id", user_id_str);
        datafile_set_col(self->requests_db, &request_book_data, "book_id", book_id_str);

        // prepare appropriate qty_requested
        sprintf(qty_requested_str, "%d", qty_requested);
        datafile_set_col(self->requests_db, &request_book_data, "qty_requested", qty_requested_str); 

        // update or add requested qty as appropriate
        if (!request_id && qty_requested > 0)
            datafile_add_row(self->requests_db, &request_book_data);
        else if (request_id && qty_requested <= 0)
            datafile_delete_row(self->requests_db, request_id);
        else
            datafile_update_row(self->requests_db, request_id, &request_book_data);

        datafile_free_row(self->requests_db, &request_book_data);
    }

    return true;
}

////
bool catalog_return_book(catalog_t *self, const char *book_name, int user_id, int qty_returning)
{
    if (self == NULL)
        return false;

    return catalog_request_book(self, book_name, user_id, -qty_returning);
}

////
int catalog_get_book_id(catalog_t *self, const char *book_name)
{
    if (self == NULL || book_name == NULL)
        return 0;

    if (strcmp(book_name, "") == 0)
        return 0;

    datafile_get_row_prepare(self->catalog_db);
    char **row = datafile_get_row_by_field(self->catalog_db, "book_name", book_name);

    int book_id = 0;

    if (row != NULL)
    {
        book_id = atoi(row[0]);
        datafile_free_row(self->catalog_db, &row);
    }
    
    return book_id;
}

////
bool catalog_book_exists(catalog_t *self, const char *book_name)
{
    if (self == NULL || book_name == NULL)
        return false;

    if (strcmp(book_name, "") == 0)
        return false;

    return catalog_get_book_id(self, book_name) > 0;
}

////
bool catalog_generate_report(catalog_t *self, const char *report_filename)
{
    if (self == NULL || report_filename == NULL)
        return false;

    FILE *fp = fopen(report_filename, "w");
    
    datafile_get_row_prepare(self->catalog_db);

    char **row;

    fprintf(fp, "%-20s%20s%20s%20s\n", "BOOK NAME", "TOTAL ON HAND", "IN USE", "AVAILABLE");
    while ((row = datafile_get_row(self->catalog_db)) != NULL)
    {
        int total_on_hand = atoi(row[datafile_get_field_index(self->catalog_db, "qty_total")]);
        int available = catalog_get_book_avail_qty(self, row[datafile_get_field_index(self->catalog_db, "book_name")]);
        int in_use = total_on_hand - available;

        fprintf(fp, "%-20s%20d%20d%20d\n", 
            row[datafile_get_field_index(self->catalog_db, "book_name")], total_on_hand, in_use, available);

        datafile_free_row(self->catalog_db, &row);
    }

    fclose(fp);

    return true;
}

////
char *catalog_get_availability_report(catalog_t *self)
{
    if (self == NULL)
        return NULL;

    int n = 2;
    char *availability_report = malloc(sizeof(char)*CATALOG_AVAIL_LINE_LEN);
    char report_line[CATALOG_AVAIL_LINE_LEN] = {0};
    char **row;

    sprintf(availability_report, "%-20s%20s%20s\n", "BOOK NAME", "TOTAL ON HAND", "AVAILABLE");

    datafile_get_row_prepare(self->catalog_db);

    while ((row = datafile_get_row(self->catalog_db)) != NULL)
    {
        int total_on_hand = atoi(row[datafile_get_field_index(self->catalog_db, "qty_total")]);
        int available = catalog_get_book_avail_qty(self, row[datafile_get_field_index(self->catalog_db, "book_name")]);

        sprintf(report_line, "%-20s%20d%20d\n", 
            row[datafile_get_field_index(self->catalog_db, "book_name")], total_on_hand, available);

        datafile_free_row(self->catalog_db, &row);

        availability_report = (char *)realloc(availability_report, (sizeof(char) * CATALOG_AVAIL_LINE_LEN * n++));

        strcat(availability_report, report_line);
    }

    return availability_report;
}