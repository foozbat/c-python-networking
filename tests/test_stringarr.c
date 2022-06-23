#include "common.h"

int main()
{
    init();

    char string[255] = "test\ttest\ttest\n";

    char **arr = record2array(string, NULL);

    free_string_array(&arr, 3);


    printf("done");

    exit(EXIT_SUCCESS);
}