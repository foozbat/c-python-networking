/*
 * AUTH CLASS PROTOTYPE
 * Author:      Aaron Bishop
 * Date:        4/17/2020
 * Description: Creates new users and authenticates existing users
 * Usage:       Instantiate with: auth_t *myauth = new_auth()
 */
#pragma once

#ifndef AUTH_H_INCLUDED
#define AUTH_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "garbagecollector.h"
#include "datafile.h"

#define AUTH_USER_DB_FILENAME "data/users.db"

// bad chars that cause a xored character to become newline, corrupting csv:
//  237#!~e[]^_Eqwy`af1-Y@AF0(pri|hdg&$%PRI\HDG645w
//  ^^^ don't use these!
#define AUTH_PASSWORD_XOR "z9xm89c+"
#define AUTH_USERNAME_LEN 11
#define AUTH_PASSWORD_LEN 8

// AUTH OBJECT
typedef struct
{
    int gc_id;

    int user_id;
    bool authenticated;

    datafile_t *user_db;
} auth_t;

// CONSTRUCTOR
auth_t *new_auth();

// DESTRUCTOR
void auth_destroy(void *);

// METHODS

// auth_new_user()
// Creates a new user with specified username, password
bool auth_new_user(auth_t *s, const char *username, const char *password);

// auth_user_exists()
// Checks if a given username already exists
bool auth_user_exists(auth_t *s, const char *username);

// auth_login()
// Authenticates a given username and password
bool auth_login(auth_t *s, const char *username, const char *password);

#endif