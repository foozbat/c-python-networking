/*
 * TCP SERVER CLASS IMPLEMENTATION
 * Author: Aaron Bishop
 * Date:   4/16/2020
 */

#include "tcpserver.h"

// EXTERNS
extern garbagecollector_t *global_gc;
extern threadcontroller_t *global_tc;

// HELPERS
int _tcpserver_initialize(tcpserver_t *self);
void *_tcpserver_listen(void *args);

// CONSTRUCTOR
tcpserver_t *new_tcpserver(int port, void *(*worker_thread)(void *))
{
    tcpserver_t *self = calloc(1, sizeof(tcpserver_t));

    if (self == NULL) {
        printf("TCP Server memory allocation failed\n");
        return NULL;
    }

    // regiser with garbage collection
    garbagecollector_register(global_gc, (void *)self, destroy_tcpserver);

    // set port
    self->port = port;
    self->worker_thread = worker_thread;

    // initialize the tcp server
    _tcpserver_initialize(self);

    // spawn thread for listener so we can have a command line interface
    threadarguments_t *thread_args = new_threadarguments();
    thread_args->arg2 = (void *)self;
    thread_create(global_tc, _tcpserver_listen, thread_args);

    //_tcpserver_listen(self);

    return self;
}

// DESTRUCTOR
void destroy_tcpserver(void *s)
{
    tcpserver_t *self = (tcpserver_t *)s;

    printf("Shutting down TCP Server\n");

    // if we have a socket we need to release it
    if (self->server_socket)
        close(self->server_socket);

    // cleanup
    garbagecollector_unregister(global_gc, self->gc_id);
        free(self);
}

// HELPERS

// Description: Internal method to create and bind the socket
int _tcpserver_initialize(tcpserver_t *self)
{
    // create the socket
    self->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (self->server_socket == -1)
    {
        exit_error("Socket creation failed");
    }

    // setup local server address struct
    self->server_addr.sin_family = AF_INET;
    self->server_addr.sin_addr.s_addr = INADDR_ANY;
    self->server_addr.sin_port = htons(self->port);

    // prevent socket bind from "sticking"
    const int       opt_val = 1;
    const socklen_t opt_len = sizeof(opt_val);
    setsockopt(self->server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_val, opt_len);

    // bind the socket
    if (bind(self->server_socket, (struct sockaddr*)&(self->server_addr), sizeof(self->server_addr)) < 0)
    {
        exit_error("Socket bind failed");        
    }

    printf("TCP Server initialized\n");
    return 1;
}

// Description: listens for connections on a specified port
// Notes:       this is a threaded function
// Arguments:   arg1: thread_id
//              arg2: self
void *_tcpserver_listen(void *args)
{
    threadarguments_t *thread_args = (threadarguments_t *)args;
    int thread_id = *(int *)(thread_args->arg1);
    tcpserver_t *self = (tcpserver_t *)(thread_args->arg2);

    int client_sock = -1;
    socklen_t sock_len = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;

    // start listening on specified port
    listen(self->server_socket, 5);
    printf("[TID: %u] Listening on port %d.\n", thread_id, self->port);

    if(fcntl(self->server_socket, F_SETFL, fcntl(self->server_socket, F_GETFL) | O_NONBLOCK) < 0)
        exit_error("Could not put socket into non-blocking mode.");

    // handle incoming connections, but cancel on kill_lock
    while (pthread_mutex_trylock(&(global_tc->kill_lock)) != 0)
    {
        client_sock = accept(self->server_socket, (struct sockaddr*)&client_addr, &sock_len);

        if (client_sock != -1)
        {
            // make copy of client addr to send to thread
            int temp_sock = client_sock;
            struct sockaddr_in temp_addr = client_addr;

            // prepare a thread for the worker        
            threadarguments_t *thread_args = new_threadarguments();
            thread_args->arg2 = (void *)&temp_sock;
            thread_args->arg3 = (void *)&temp_addr;
            thread_create(global_tc, self->worker_thread, thread_args);
        }
    }

    printf("[TID: %u] Closing listener.\n", thread_id);
    pthread_mutex_unlock(&(global_tc->kill_lock));

    return NULL;
}