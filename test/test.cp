#include <stdio.h>
#include <stdlib.h>

int main()
{
    char d[3] = {'0', '1', '\0'};
    char m[3] = {'1', '1', '\0'};
    char y[3] = {'2', '4', '\0'};

    // printf("%d-%d-20%d", atoi(d), atoi(m), atoi(y));
    printf("%s-%s-20%s", d, m, y);

    return 0;
}
