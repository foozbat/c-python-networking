#include "common.h"
#include "catalog.h"

int main()
{
    printf("starting catalog unit test\n");

    init();

    catalog_t *catalog = new_catalog();

    catalog_add_book(catalog, "The Best Book Ever 4", 5);


    int qty_available;
    qty_available = catalog_get_book_avail_qty(catalog, "The Best Book Ever 3");

    printf("qty available is: %d\n", qty_available);
    catalog_request_book(catalog, "The Best Book Ever 3", 1, 5);
    qty_available = catalog_get_book_avail_qty(catalog, "The Best Book Ever 3");

    printf("qty available after requested: %d\n", qty_available);
    catalog_return_book(catalog, "The Best Book Ever 3", 1, 2);

    qty_available = catalog_get_book_avail_qty(catalog, "The Best Book Ever 3");
    printf("qty available after return: %d\n", qty_available);

    printf("ending unit test\n");


    
    char *availability_report = catalog_get_availability_report(catalog);

    //if (availability_report != NULL && sz > 0)
        printf("%s", availability_report);

        free(availability_report);
   // else
        //printf("availability: %d, sz: %ld\n", (int)availability_report, sz);


    //catalog_generate_report(catalog, "reports/something.txt");
    exit(EXIT_SUCCESS);
}