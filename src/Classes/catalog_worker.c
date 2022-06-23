/*
    CATALOG WORKER IMPLEMENTATION (threaded)
    Author: Aaron Bishop
    Date:   4/19/2020
*/

#include "catalog_worker.h"

extern threadcontroller_t *global_tc;

/////
void *catalog_worker(void *args)
{
    // thread argument input
    threadarguments_t *thread_args = (threadarguments_t *)args;
    int thread_id = *(int *)(thread_args->arg1);
    int client_sock = *(int *)(thread_args->arg2);
    struct sockaddr_in client_addr = *(struct sockaddr_in *)(thread_args->arg3);

    int bytes_received = -1;
    bool adding_user = 0;
    char new_username[AUTH_USERNAME_LEN+1] = {0}; // need to store this out here since spec wants separate packets for username/password

    // catalog specific objects
    auth_t *auth = new_auth();
    catalog_t *catalog = new_catalog();

    // no operation in the protocol exceeds 20 bytes
    char buffer[CATALOG_CMD_MAXLEN] = {0};

    printf("[TID: %u] Connection accepted from: %s:%d\n", thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if(fcntl(client_sock, F_SETFL, fcntl(client_sock, F_GETFL) | O_NONBLOCK) < 0)
        exit_error("Could not put socket into non-blocking mode.");

    // receive data from client, but cancel on kill lock
    //   instead of using a blocking recv call, we'll use a while loop with a kill_lock
    //   to enable graceful shutdown
    while (pthread_mutex_trylock(&(global_tc->kill_lock)) != 0) // change to check for kill lock
    {
        // receive incoming command
        bytes_received = recv(client_sock, buffer, CATALOG_CMD_MAXLEN, 0);

        if (bytes_received == 0)
        {
            printf("[TID: %u] Disconnect received from client: %s:%d.\n", 
                thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            close(client_sock);
            return NULL;
        }
        else if (bytes_received > 0)
        {
            int response_code = 0;

            // parse out op_code
            uint32_t op_code = 0;
            memcpy(&op_code, buffer, sizeof(char));

            // COMMAND: CONNECT - complete
            /////
            if (op_code == CATALOG_CMD_CONNECT)
            {
                char response[3][CATALOG_CMD_MAXLEN] = 
                {
                    "invalid_user", 
                    "invalid_password", 
                    "login_success"
                };

                if (bytes_received == 20)
                {
                    char username[AUTH_USERNAME_LEN+1] = {0};
                    char password[AUTH_PASSWORD_LEN+1] = {0};

                    memcpy(&username, buffer+sizeof(char), sizeof(char)*AUTH_USERNAME_LEN);
                    memcpy(&password, buffer+sizeof(char)+sizeof(char)*AUTH_USERNAME_LEN, sizeof(char)*AUTH_PASSWORD_LEN);
                    memset(buffer, 0, sizeof(buffer));

                    xor_crypt(password, AUTH_PASSWORD_XOR);

                    if (!auth_user_exists(auth, username))
                        response_code = 0;
                    else if (!auth_login(auth, username, password))
                        response_code = 1;
                    else
                        response_code = 2;
                    
                    printf("[TID: %u] Login attempt from %s:%d, username: %s, %s\n", 
                        thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username, response[response_code]);
                    
                    send(client_sock, response[response_code], strlen(response[response_code]), 0);
                }
            }

            // Note: All remaining commands require user to be logged in
            if (auth->authenticated)
            {
                // COMMAND ADD USER - complete
                /////
                if (op_code == CATALOG_CMD_ADD_USER)
                {
                    response_code = 0;
                    char response[4][CATALOG_CMD_MAXLEN] = 
                    {
                        "username_exists",
                        "username_available",
                        "add_user_failed",
                        "add_user_success"
                    };

                    if (bytes_received <= 12)
                    {
                        char password[AUTH_PASSWORD_LEN+1] = {0};
                        
                        if (!adding_user)
                        {
                            memcpy(&new_username, buffer+sizeof(char), sizeof(char)*AUTH_USERNAME_LEN);
                            memset(buffer, 0, sizeof(buffer));

                            if (auth_user_exists(auth, new_username))
                            {
                                response_code = 0;
                            }
                            else
                            {
                                response_code = 1;
                                adding_user = 1;
                            }
                        }
                        else if (adding_user)
                        {
                            // read second packet to get password
                            char new_password[AUTH_PASSWORD_LEN+1] = {0};
                            memcpy(&new_password, buffer+sizeof(char), sizeof(char)*8);

                            xor_crypt(new_password, AUTH_PASSWORD_XOR);

                            if (auth_new_user(auth, new_username, new_password))
                                response_code = 3;
                            else
                                response_code = 2;
                            
                            adding_user = 0;
                            memset(new_username, 0, sizeof(new_username));

                        }
                        send(client_sock, response[response_code], strlen(response[response_code]), 0);

                        printf("[TID: %u] Add user attempt from %s:%d, username: %s, %s\n", 
                            thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_username, response[response_code]);
                    }
                }
                // COMMAND ADD BOOK - complete
                /////
                if (op_code == CATALOG_CMD_ADD_BOOK)
                {
                    //printf("Add Book Command\n");

                    response_code = 0;
                    
                    char response[2][CATALOG_CMD_MAXLEN] = 
                    {
                        "add_book_error", 
                        "add_book_success", 
                    };

                    uint16_t num_books = 0;
                    char book_name[CATALOG_BOOK_NAME_LEN+1] = {0};

                    memcpy(&num_books, buffer+sizeof(char), sizeof(uint16_t));
                    memcpy(&book_name, buffer+sizeof(char)+sizeof(uint16_t), sizeof(char)*CATALOG_BOOK_NAME_LEN);
                    memset(buffer, 0, sizeof(buffer));

                    num_books = ntohs(num_books);

                    response_code = catalog_add_book(catalog, book_name, (int)num_books);

                    printf("[TID: %u] Add book attempt from %s:%d, book_name: %s, qty: %d, %s\n", 
                        thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), book_name, num_books, response[response_code]);
                    send(client_sock, response[response_code], strlen(response[response_code]), 0);
                }
                // COMMAND REQUEST BOOK - complete
                /////
                if (op_code == CATALOG_CMD_REQUEST_BOOK)
                {
                    char response[2][CATALOG_CMD_MAXLEN] = 
                    {
                        "req_book_error",
                        "req_book_success"
                    };

                    uint16_t num_books = 0;
                    char book_name[CATALOG_BOOK_NAME_LEN+1] = {0};

                    memcpy(&num_books, buffer+sizeof(char), sizeof(uint16_t));
                    memcpy(&book_name, buffer+sizeof(char)+sizeof(uint16_t), sizeof(char)*CATALOG_BOOK_NAME_LEN);
                    memset(buffer, 0, sizeof(buffer));

                    num_books = ntohs(num_books);

                    response_code = catalog_request_book(catalog, book_name, auth->user_id, (int)num_books);

                    printf("[TID: %u] Request book attempt from %s:%d, book_name: %s, qty: %d, %s\n", 
                        thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), book_name, num_books, response[response_code]);
                    send(client_sock, response[response_code], strlen(response[response_code]), 0);
                }
                // COMMAND RETURN BOOK - complete
                /////
                if (op_code == CATALOG_CMD_RETURN_BOOK)
                {
                    char response[2][CATALOG_CMD_MAXLEN] = 
                    {
                        "ret_book_error",
                        "ret_book_success"
                    };

                    uint16_t num_books = 0;
                    char book_name[CATALOG_BOOK_NAME_LEN+1] = {0};

                    memcpy(&num_books, buffer+sizeof(char), sizeof(uint16_t));
                    memcpy(&book_name, buffer+sizeof(char)+sizeof(uint16_t), sizeof(char)*CATALOG_BOOK_NAME_LEN);
                    memset(buffer, 0, sizeof(buffer));

                    num_books = ntohs(num_books);

                    response_code = catalog_return_book(catalog, book_name, auth->user_id, (int)num_books);

                    printf("[TID: %u] Return book attempt from %s:%d, book_name: %s, qty: %d, %s\n", 
                        thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), book_name, num_books, response[response_code]);
                    send(client_sock, response[response_code], strlen(response[response_code]), 0);
                }
                // COMMAND REQUEST REPORT
                /////
                if (op_code == CATALOG_CMD_GET_AVAILABILITY)
                {
                    char *availability_report = catalog_get_availability_report(catalog);

                    printf("[TID: %u] Get availability request from %s:%d\n", 
                        thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    send(client_sock, availability_report, strlen(availability_report), 0);

                    free(availability_report);
                }
                // COMMAND REQUEST REPORT
                /////
                if (op_code == CATALOG_CMD_REQUEST_REPORT)
                {
                    char response[3][CATALOG_CMD_MAXLEN] = 
                    {
                        "req_report_error",
                        "req_filesize_error",
                        "req_report_success"
                    };

                    uint16_t listener_port = 0;
                    memcpy(&listener_port, buffer+sizeof(char), sizeof(uint16_t));
                    memset(buffer, 0, sizeof(buffer));

                    listener_port = ntohs(listener_port);

                    printf("[TID: %u] Request report from %s:%d, listener port %d\n", 
                            thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), listener_port);

                    // generate report
                    // format current time
                    char date[30];
                    char report_filename[256] = {0};
                    time_t now = time(NULL);
                    struct tm *t = localtime(&now);

                    strftime(date, sizeof(date)-1, "%Y%m%d_%H%M%S", t);
                    sprintf(report_filename, "reports/inventory_report_%d_%s.txt", auth->user_id, date);

                    bool report_complete = catalog_generate_report(catalog, report_filename);

                    // send report to listener on client
                    if (report_complete)
                    {
                        printf("[TID: %u]   Report generated, sending to listener...\n", thread_id);

                        // connect to client listener
                        struct sockaddr_in client_listener = client_addr;
                        client_listener.sin_port = htons(listener_port);

                        int client_listen_sock = socket(AF_INET, SOCK_STREAM, 0);

                        if (client_listen_sock == -1)
                            continue;

                        if (connect(client_listen_sock, (struct sockaddr *)&client_listener, sizeof(struct sockaddr_in)) < 0)
                            continue;

                        // load the completed report into memory
                        FILE *fp = fopen(report_filename, "rb");
                        fseek(fp, 0, SEEK_END);
                        long file_size = ftell(fp);
                        fseek(fp, 0, SEEK_SET);

                        char *file_buffer = malloc(file_size);
                        fread(file_buffer, 1, file_size, fp);
                        fclose(fp);

                        // send the completed report
                        send(client_listen_sock, file_buffer, file_size, 0);

                        free(file_buffer);

                        // get ack from listener
                        bytes_received = recv(client_listen_sock, buffer, CATALOG_CMD_MAXLEN, 0);

                        if (bytes_received > 0)
                        {
                            uint32_t received_file_size = 0;
                            memcpy(&received_file_size, buffer, sizeof(uint32_t));
                            received_file_size = ntohl(received_file_size);

                            printf("[TID: %u]   Sent %ld bytes, client acked with %u\n", thread_id, file_size, received_file_size); 

                            // compare acknowledgement to file size
                            if (received_file_size == file_size)
                                response_code = 2;
                            else
                                response_code = 1;
                        }
                        else
                        {
                            response_code = 0;
                        }

                        send(client_listen_sock, response[response_code], strlen(response[response_code]), 0);
                        close(client_listen_sock);

                    }
                }
                // END COMMANDS

            } // end authenticated options
        } // endif bytes_received
    } // end while

    pthread_mutex_unlock(&(global_tc->kill_lock));

    printf("[TID: %u] Closing connection with client %s:%d.\n", thread_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    close(client_sock);

    return NULL;
}