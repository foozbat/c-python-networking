/*
 * AUTH CLASS IMPLEMENTATION
 * Author: Aaron Bishop
 * Date:   4/17/2020
 */

#include "auth.h"

extern garbagecollector_t *global_gc;

////
auth_t *new_auth()
{
    auth_t *self = calloc(1, sizeof(auth_t));

    if (self == NULL)
        exit_error("Auth memory allocation failed");

    garbagecollector_register(global_gc, (void *)self, auth_destroy);

    self->user_db = new_datafile(AUTH_USER_DB_FILENAME);

    if (self->user_db == NULL)
        exit_error("failed to initialize user database");

    return self;
}

////
void auth_destroy(void *s)
{
    auth_t *self = (auth_t *)s;
    garbagecollector_unregister(global_gc, self->gc_id);
    free(self);
}

////
bool auth_new_user(auth_t *self, const char *username, const char *password)
{
    if (username == NULL || password == NULL)
        return false;

    // prevent duplicate user registration
    if (auth_user_exists(self, username))
        return false;

    char **user_data = datafile_new_row_array(self->user_db);
    datafile_set_col(self->user_db, &user_data, "username", username);
    datafile_set_col(self->user_db, &user_data, "password", password);
    datafile_add_row(self->user_db, &user_data);
    datafile_free_row(self->user_db, &user_data);

    return true;
}

////
bool auth_user_exists(auth_t *self, const char *username)
{
    //printf("auth_user_exists()\n");
    if (self == NULL || username == NULL)
        return false;

    if (strcmp(username, "") == 0)
        return false;

    datafile_get_row_prepare(self->user_db);
    char **user_row = datafile_get_row_by_field(self->user_db, "username", username);
    if (user_row == NULL)
    {
        return false;
    }
    else
    {
        datafile_free_row(self->user_db, &user_row);
        return true;
    }
}

////
bool auth_login(auth_t *self, const char *username, const char *password)
{
    if (username == NULL || password == NULL)
        return 0;

    bool login_success = false;

    if (!auth_user_exists(self, username))
        return false; // possibly change

    // check password
    datafile_get_row_prepare(self->user_db);
    char **user_row = datafile_get_row_by_field(self->user_db, "username", username);

    if (strcmp(user_row[datafile_get_field_index(self->user_db, "password")], password) == 0)
    {
        self->user_id = atoi(user_row[0]);
        self->authenticated = true;
    }
    datafile_free_row(self->user_db, &user_row);

    return self->authenticated;
}
