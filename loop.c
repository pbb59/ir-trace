#include <stdio.h>
#include <stdbool.h>

unsigned int add4(unsigned int x, bool en) {
    if(en)
        return x + 4;
    else
        return x;
}

unsigned int loop(unsigned int LIMIT) {
    unsigned int x = 0;
    for(int i = 0; i < LIMIT; i++) {
        bool option = false;
        if(i%2 == 0) {
            option = true;
        }
        x = add4(x, option);
    }
    return x;
}

int main() {
    printf("Output: %d\n", loop(10));
    return 1;
}
