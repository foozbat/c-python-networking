#include "common.h"

int main()
{
    init();

    char chars[] = "`1234567890-=~!@#$%^&*()_+qwertyuiop[]\\QWERTYUIOP{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?}";
    char xored[] = "`1234567890-=~!@#$%^&*()_+qwertyuiop[]\\QWERTYUIOP{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?}";

    int len = strlen(chars);
    for (int i=0; i<len; i++)
    {
        char my_xor[2] = {0};
        my_xor[0] = chars[i];
        for (int j=0; j<256; j++)
        {
            char key[2] = {0};
            key[0] = chars[j];

            xor_crypt(my_xor, key);

            if (my_xor[0] == '\n')
            {
                //printf("%d: (%c) ^ (%c) = (%c)\n", i, chars[i], key[0], my_xor[0]);
                printf("%c", key[0]);
            }

            xor_crypt(my_xor, key);
        }
    }

    printf("\n");

    exit(EXIT_SUCCESS);
}