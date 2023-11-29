#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <lib/randjson.h>

int main()
{
    int i;
    JsonGenerator JG;
    JG.max_array_len = 5;
    JG.max_recursion = 3;
    JG.max_string_len = 30;
    for (i = 0; i < 1; i++) {
        char* s = randjson_make_json(clock(), &JG);
        printf("%s\n",s);
        free(s);
    }
    exit(0);
}
