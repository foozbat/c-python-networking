/*
 * CAPSTONE SERVER MAIN DRIVER
 * Author: Aaron Bishop
 * Date:   4/16/2020
 */

#include "common.h"
#include "tcpserver.h"
#include "catalog_worker.h"

// possibly change this to arg
#define DEFAULT_PORT 31337

int main(int argc, char *argv[])
{
    init();

    // create the tcp server and pass it our worker to handle catalog commands
    tcpserver_t *server = new_tcpserver(DEFAULT_PORT, catalog_worker);

    sleep(0.5);

    // handle user input
    printf("Type 'q' to exit.\n");

    char c;

    while((c = getchar()) != 0)
    {
        // q is the command to quit... AMAZING
        if (c == 'q')
            break;
    }

    exit(EXIT_SUCCESS);
}