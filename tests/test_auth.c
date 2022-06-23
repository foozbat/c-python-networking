#include "common.h"
#include "auth.h"

int main()
{
    printf("starting auth unit test\n");

    init();

    // csv test
    auth_t *auth = new_auth();

    char test_username[255];
    time_t t;
    srand((unsigned) time(&t));
    
    sprintf(test_username, "test\t\t\t%d%d", rand()%99, rand()%99);

    printf("adding new user\n");
    
    auth_new_user(auth, test_username, "pass\tpass");

    printf("login checks\n");
    printf("login success: %d\n", auth_login(auth, "awesomeuser", "awesomepass"));
    printf("login success: %d\n", auth_login(auth, "test1", "testz"));
    printf("login success: %d\n", auth_login(auth, "test3168", "testpw"));

    

    /*
    char find_user1[] = "test2";
    char find_user2[] = "nobody";

    printf("exists? %d\n", auth_user_exists(auth, "test2"));
    printf("exists? %d\n", auth_user_exists(auth, "nobody"));
    */

    printf("ending unit test\n");
    exit(EXIT_SUCCESS);
}