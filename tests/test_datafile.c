#include "common.h"
#include "datafile.h"

int main()
{
    printf("starting datafile unit test\n");

    init();

    datafile_t *df = new_datafile("data/test.db");


    char **row_data = datafile_new_row_array(df);

    datafile_set_col(df, row_data, "field1", "UPDATEasdf1");
    datafile_set_col(df, row_data, "field2", "UPDATEasdf2");
    datafile_set_col(df, row_data, "field3", "UPDATEasdf3");

    datafile_update_row(df, 4, row_data);
    //datafile_add_row(df, row_data);

    datafile_get_row_prepare(df);
    char **row_data2 = datafile_get_row_by_field(df, "field1", "asdf1z");

    printf("gothere\n");

    if (row_data2 != NULL)
        for (int i=0; i<df->num_fields; i++)
            printf("%s,", row_data[i]);
    else
        printf("no result");
    printf("\n");

/*
    char test1[255] = "test11,test12,test13,test14";
    char test2[255] = "text21,,test23,test24";
    char test3[255] = ",,test,test,test";
    char test4[255] = "test,test,test,,";
    char test5[255] = ",,,,";

    int n1;
    char **test_row1 = csv2array(test1, &n1);
    int n2;
    char **test_row2 = csv2array(test2, &n2);
    
    printf("csv2array: n: %d, %s,%s,%s,%s\n", n1, test_row1[0], test_row1[1], test_row1[2], test_row1[3]);
    printf("csv2array: n: %d, %s,%s,%s,%s\n", n2, test_row2[0], test_row2[1], test_row2[2], test_row2[3]);
    //printf("csv2array: n: %d\n", n);
*/

    printf("ending unit test\n");
    exit(EXIT_SUCCESS);
}