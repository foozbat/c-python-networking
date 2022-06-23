/*
    CATALOG WORKER FUNCTION (threaded)
    Author:      Aaron Bishop
    Date:        4/19/2020
    Description: Imperitive style function to encapsualte all client specific objects within a common scope
                   Worker implements the CATALOG protocol 
    Usage:       Pass this function to the tcpserver object and it will be run for each accepted connection

    Note: Capstone specification requires minimum of 10 workers.  This implementation support N workers up to the
          total limit of threads as defined in threadcontroller.h.

*/

#pragma once

#ifndef CATALOG_WORKER_H_INCLUDED
#define CATALOG_WORKER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "threadcontroller.h"
#include "auth.h"
#include "catalog.h"

#define CATALOG_CMD_MAXLEN 20

#define CATALOG_CMD_CONNECT 0x10
#define CATALOG_CMD_ADD_USER 0x20
#define CATALOG_CMD_REQUEST_BOOK 0x30
#define CATALOG_CMD_ADD_BOOK 0x40
#define CATALOG_CMD_REQUEST_REPORT 0x50
#define CATALOG_CMD_RETURN_BOOK 0x60
#define CATALOG_CMD_GET_AVAILABILITY 0x70

// catalog_worker()
// Threaded worker function which handles all commands from a connected client
void *catalog_worker(void *args);

#endif