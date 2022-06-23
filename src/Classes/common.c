 /*
 * COMMON FUNCTIONS
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Procedural functions accessible to all methods and drivers
 */

#include <string.h>

#include "common.h"

// create globally accessible objects
garbagecollector_t *global_gc = NULL;
threadcontroller_t *global_tc = NULL;
devlog_t *global_dl;

// called by atexit()
void _cleanup() {
    garbagecollector_cleanup(global_gc);

    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
}

/////
void init()
{
    // create global objects
    global_gc = new_garbagecollector();
    global_tc = new_threadcontroller();
    global_dl = new_devlog("test.txt");
    
    // handle various types of signals so we can collect garbage gracefully
    struct sigaction sa; 
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &exit;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    // register cleanup function
    atexit(_cleanup);
}

/////
void exit_error(const char *error)
{
    printf("Terminating Program\n");
    printf("Error: %s\n", error);
    exit(0);
}

/////
char **record2array(char *record_str, int *num_fields)
{
    int num_fields_temp = 0;

    if (num_fields == NULL)
        num_fields = &num_fields_temp;
    else
        *num_fields = 0;

    char *record_tok = calloc(strlen(record_str)+1, sizeof(char));
    strcpy(record_tok, record_str);

    // build array from fields
    record_tok[strcspn(record_tok, "\n")] = 0; // strip newline
    char *field = strtok(record_tok, "\t");
    char **ret_array = NULL;

    while (field)
    {
        ret_array = realloc(ret_array, sizeof(char *) * ((*num_fields)+1));

        ret_array[*num_fields] = calloc((strlen(field)+1), sizeof(char));

        strcpy(ret_array[*num_fields], field);

        field = strtok(NULL, "\t");
        (*num_fields)++;
    }

    free(record_tok);

    return ret_array;
}

/////
char **new_string_array(int num_strings)
{
    if (num_strings == 0)
        return NULL;

    // initialize array of strings to array of null pointers
    char **ret_array = calloc(num_strings, sizeof(char *));

    return ret_array;
}

/////
char *array2record(char **arr, int num_fields)
{
    char *ret_str = calloc(1, sizeof(char));
    int len = 0;

    for (int i=0; i<num_fields; i++)
    {
        if (arr[i] != NULL)
        {
            strip_tabs(&arr[i]);
            len = len + strlen(arr[i]) + 2;
            ret_str = realloc(ret_str, len);
            if (i==0)
                strcpy(ret_str, arr[i]);
            else
                strcat(ret_str, arr[i]);
            if (i+1 < num_fields)
                strcat(ret_str, "\t");
            else
                strcat(ret_str, "\n");
        }
    }

    return ret_str;
}

/////
void strip_tabs(char **string)
{
    for (unsigned int i=0; i<strlen(*string); i++)
        if ((*string)[i] == '\t')
            (*string)[i] = ' ';
}

/////
void free_string_array(char ***arr, int n)
{
    if (arr == NULL)
        return;

    //printf("n is %d\n", n);

    for (int i=0; i<n; i++)
    {
        //printf("i = %d\n", i);
        if ((*arr)[i] != NULL)
        {
            //printf("freeing i...\n");
            free((*arr)[i]);
        }
    }

    //printf("freeing arr...\n");
    free(*arr);
}

/////
bool file_exists(const char *filename)
{
    return access(filename, F_OK) != -1;
}

/////
int strpos(char *string, char *search)
{
    char *p = strstr(string, search);
    if (p)
        return p - string;
    return -1;
}

/////
void xor_crypt(char *string, const char *key)
{
    int len = strlen(string);
    for(int i=0; i<len; i++)
    {
        string[i]=string[i]^key[i];
    }
}