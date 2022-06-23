/*
    DATAFILE CLASS PROTOTYPE
    Author:      Aaron Bishop
    Date:        4/17/2020
    Description: Abstraction for working with tab delimitted text data files
    Usage:       Instantiate with: datafile_t *mydatafile = new_datafile()
*/

#include "datafile.h"

extern garbagecollector_t *global_gc;

// CONSTRUCTOR
datafile_t *new_datafile(const char *filename)
{
    if (filename == NULL)
        return NULL;
    if (strcmp(filename, "") == 0)
        return NULL;
    if (!file_exists(filename))
        return NULL;

    datafile_t *self = calloc(1, sizeof(datafile_t));

    if (self == NULL)
        exit_error("Datafile memory allocation failed\n");

    garbagecollector_register(global_gc, (void *)self, datafile_destroy);

    strcpy(self->filename, filename);

    // get the header data
    char header[1024];
    FILE *fp = fopen(filename, "r");
    fgets(header, 1024, fp);
    fclose(fp);

    // read in field names from header
    self->header_len = strlen(header);
    if (self->header_len < 1)
        exit_error("invalid database file");
    self->field_names = record2array(header, &(self->num_fields));
    self->num_data_fields = self->num_fields - 3;

    self->last_row_id = 0;

    return self;
}

// DESTRUCTOR
void datafile_destroy(void *s)
{
    if (s == NULL)
        return;

    datafile_t *self = (datafile_t *)s;
    garbagecollector_unregister(global_gc, self->gc_id);
    free_string_array(&(self->field_names), self->num_fields);
    free(self);
}

// HELPERS

/////
bool _datafile_row_exists(datafile_t *self, int id)
{
    if (self == NULL)
        return false;

    bool row_exists = false;
    char buffer[DATAFILE_ROW_MAXLEN];

    FILE *fp = fopen(self->filename, "r");
    fseek(fp, self->header_len, SEEK_SET);

    while (fgets(buffer, DATAFILE_ROW_MAXLEN, fp) != NULL)
    {
        char **row = record2array(buffer, NULL);
        int current_id = atoi(row[0]);
        datafile_free_row(self, &row);
        if (current_id == id)
        {
            row_exists = true;
            break;
        }
    }

    fclose(fp);

    return row_exists;
}

////
bool _datafile_copy_file_data(const char *source, const char *dest, unsigned long offset, unsigned long sz)
{
    //printf ("copying file data\n");
    if (source == NULL || dest == NULL)
        return false;

    FILE *source_fp = fopen(source, "r");
    FILE *dest_fp = fopen(dest, "w");
    flock(fileno(dest_fp), LOCK_EX);

    char buffer[CHUNK_SIZE];
    unsigned long bytes_read    = 0;
    unsigned long bytes_to_copy = sz;
    unsigned long chunk_to_read = CHUNK_SIZE;
    unsigned long file_size;

    fseek(source_fp, 0, SEEK_END);
    file_size = ftell(source_fp);

    // check for invalid copy length
    if (offset + sz > file_size)
        return false;

    // move to specified offset
    if (fseek(source_fp, offset, SEEK_SET) != 0)
        return false;

    // if specified size is zero, write the entire file from offset
    if (sz == 0)
        bytes_to_copy = file_size - offset;

    // don't read more bytes than there are bytes available
    if (bytes_to_copy < chunk_to_read)
        chunk_to_read = bytes_to_copy;

    while ((bytes_read = fread(buffer, 1, chunk_to_read, source_fp)) != 0 && bytes_to_copy > 0)
    {
        if(fwrite(buffer, 1, bytes_read, dest_fp) != bytes_read)
            return false;

        bytes_to_copy -= bytes_read;
    }

    fclose(source_fp);
    flock(fileno(dest_fp), LOCK_UN);
    fclose(dest_fp);
    return true;
}

// METHODS

/////
bool datafile_add_row(datafile_t *self, char ***row_data)
{
    if (self == NULL)
        return false;

    //printf("datafile_add_row()\n");
    char buffer[DATAFILE_ROW_MAXLEN];
    char date_added[100];
    int last_id = 0;

    // format current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date_added, sizeof(date_added)-1, "%Y-%m-%d %H:%M:%S", t);

    FILE *fp = fopen(self->filename, "r+");
    flock(fileno(fp), LOCK_EX);
    fseek(fp, self->header_len, SEEK_SET);

    // get last id
    fseek(fp, self->header_len, SEEK_SET);
    while (fgets(buffer, DATAFILE_ROW_MAXLEN, fp) != NULL)
    {
        char **row = record2array(buffer, NULL);
        last_id = atoi(row[0]);
        datafile_free_row(self, &row);
    }

    // insert new row
    char new_id[255];
    sprintf(new_id, "%d", last_id+1);

    datafile_set_col(self, row_data, "id", new_id);
    datafile_set_col(self, row_data, "date_created", date_added);
    datafile_set_col(self, row_data, "date_updated", date_added);

    char *new_row = array2record(*row_data, self->num_fields);

    fputs(new_row, fp);
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    
    free(new_row);

    return false;
}

////
bool datafile_update_row(datafile_t *self, int id, char ***row_data)
{
    if (self == NULL)
        return false;
    if (!_datafile_row_exists(self, id))
        return false;

    char buffer[DATAFILE_ROW_MAXLEN];
    char temp_file_name[50] = {0};
    time_t t;
    srand((unsigned) time(&t));

    sprintf(temp_file_name, "/tmp/datafile_temp%d%d%d%d", rand()%99, rand()%99, rand()%99, rand()%99);

    FILE *fp = fopen(self->filename, "r");
    FILE *fp_temp = fopen(temp_file_name, "w");

    // write header first
    fgets(buffer, DATAFILE_ROW_MAXLEN, fp);
    fputs(buffer,fp_temp);

    while (fgets(buffer, DATAFILE_ROW_MAXLEN, fp) != NULL)
    {
        int n;
        char **update_row = record2array(buffer, &n);

        int current_id = atoi(update_row[0]);

        if (current_id != id)
        {
            // write the existing row to temp file
            fputs(buffer,fp_temp);
        }
        // if this is the row to update...
        else if (row_data != NULL) // implicitly delete the row if row_data not provided.
        {
            // format current time
            char date_updated[100];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            strftime(date_updated, sizeof(date_updated)-1, "%Y-%m-%d %H:%M:%S", t);

            //datafile_set_col(self, row_data, "id", existing_row[0]);
            //datafile_set_col(self, row_data, "date_created", existing_row[1]);
            datafile_set_col(self, &update_row, "date_updated", date_updated);

            // update only colums specified in row_data
            for (int i=3; i<self->num_fields; i++)
            {
                //const char *
                if ((*row_data)[i] != NULL)
                    datafile_set_col(self, &update_row, self->field_names[i], (*row_data)[i]);
                //else
                //    datafile_set_col(self, row_data, self->field_names[i], existing_row[i]);
            }

            char *updated_row_str = array2record(update_row, self->num_fields);
            fputs(updated_row_str, fp_temp);
            free(updated_row_str);
        }

        datafile_free_row(self, &update_row);
    }

    fclose(fp);
    fclose(fp_temp);

    _datafile_copy_file_data(temp_file_name, self->filename, 0, 0);

    return true;
}

////
bool datafile_delete_row(datafile_t *self, int id)
{
    if (self == NULL)
        return false;

    return datafile_update_row(self, id, NULL);
}

////
void datafile_get_row_prepare(datafile_t *self)
{
    if (self == NULL)
        return;

    self->last_row_id = 0;
}

////
char **datafile_get_row(datafile_t *self)
{
    if (self == NULL)
        return NULL;

    char **ret_row = NULL;
    char buffer[DATAFILE_ROW_MAXLEN];

    FILE *fp = fopen(self->filename, "r");
    fseek(fp, self->header_len, SEEK_SET);

    while (fgets(buffer, DATAFILE_ROW_MAXLEN, fp) != NULL)
    {
        int n;
        char **current_row = record2array(buffer, &n);
        
        if (atoi(current_row[0]) > self->last_row_id)
        {
            ret_row = current_row;
            self->last_row_id = atoi(current_row[0]);
            break;
        }
        datafile_free_row(self, &current_row);
    }
    fclose(fp);
    return ret_row;
}

////
char **datafile_get_row_by_field(datafile_t *self, const char *field_name, const char *field_value)
{
    if (self == NULL || field_name == NULL || field_value == NULL)
        return NULL;
    
    // datafile doesn't support empty fields ATM
    if (strcmp(field_name, "") == 0 || strcmp(field_value, "") == 0)
        return NULL;

    char **ret_row = NULL;
    char buffer[DATAFILE_ROW_MAXLEN];

    FILE *fp = fopen(self->filename, "r");
    fseek(fp, self->header_len, SEEK_SET);

    while (fgets(buffer, DATAFILE_ROW_MAXLEN, fp) != NULL)
    {
        int n;
        char **current_row = record2array(buffer, &n);
        if (strcmp(current_row[datafile_get_field_index(self, field_name)], field_value) == 0 && atoi(current_row[0]) > self->last_row_id)
        {
            ret_row = current_row;
            self->last_row_id = atoi(current_row[0]);
            break;
        }

        datafile_free_row(self, &current_row);
    }

    fclose(fp);

    return ret_row;
}

////
char **datafile_new_row_array(datafile_t *self)
{
    if (self == NULL)
        return NULL;

    return new_string_array(self->num_fields);
}

////
bool datafile_set_col(datafile_t *self, char ***row_data, const char *field_name, const char *col_data)
{
    if (self == NULL)
        return false;

    int index = datafile_get_field_index(self, field_name);
    
    if (index < 0)
        return false;

    // if overwriting this element, free old data
    if ((*row_data)[index] != NULL)
        free((*row_data)[index]);

    (*row_data)[index] = malloc(sizeof(char) * (strlen(col_data)+1));
    strcpy((*row_data)[index], col_data);
    
    return true;
}

////
int datafile_get_field_index(datafile_t *self, const char *field_name)
{
    if (self == NULL)
        return -1;

    if (field_name == NULL)
        return -1;

    if (strcmp(field_name, "") == 0)
        return -1;

    int index = -1;

    for (int i=0; i<self->num_fields; i++)
        if (strcmp(self->field_names[i], field_name) == 0)
            index = i;

    return index;
}

////
void datafile_free_row(datafile_t *self, char ***row_data)
{
    if (self == NULL)
        return;

    free_string_array(row_data, self->num_fields);
}
