#include <stdio.h>
#include <stdbool.h>

unsigned int addop(unsigned int x, bool en, unsigned int op) {
    if(en)
        return x + op;
    else
        return x;
}

unsigned int sub2(unsigned int x, bool en) {
    if(en)
        return x - 2;
    else
        return x;
}

unsigned int loop(unsigned int LIMIT) {
    unsigned int x = 0;
    unsigned int op = 0;
    for(int i = 0; i < LIMIT; i++) {
        bool option = false;
        printf("A-");
        if(i%2 == 0) {
            option = true;
            printf("B-");
        }
        if(i > 5) {
            op = 4;
            printf("C-");
        }
        op += 2;
        printf("D-");

        if(x > 2) {
            x = sub2(x, option);
            printf("E-");
        }
        x = addop(x, option, op);
        printf("F-\n");
    }
    return x;
}

int main() {
    printf("Output: %d\n", loop(10));
    return 1;
}
