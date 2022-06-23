/*
 * TCP SERVER CLASS PROTOTYPE
 * Author:      Aaron Bishop
 * Date:        4/16/2020
 * Description: Multithreaded class to encapsulate all TCP server functionality
 * Usage:       Instantiate with: tcpserver_t *myserver = new_tcpserver()
 */
#pragma once

#ifndef TCPSERVER_H_INCLUDED
#define TCPSERVER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "common.h"
#include "garbagecollector.h"
#include "threadcontroller.h"

#define SUCCESS 1
#define ERR_SOCK_CREATION_FAILURE -1
#define ERR_BIND_FAILURE -2
#define ERR_SOCK_ACCEPT_FAILURE -3

// TCPSERVER OBJECT
typedef struct
{
    int gc_id;
    struct sockaddr_in server_addr;
    int port;
    int server_socket;
    void *(*worker_thread)(void *);

} tcpserver_t;

// CONSTRUCTOR
tcpserver_t *new_tcpserver(int port, void *(*worker_thread)(void *));

// DESTRUCTOR
void destroy_tcpserver(void *);

// METHODS

// no public methods

#endif