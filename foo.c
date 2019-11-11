#include <stdio.h>
#include <stdbool.h>

unsigned int add4(unsigned int x, bool en) {
    if(en)
        return x + 4;
    else
        return x;
}

int main() {
    printf("Output: %d\n", add4(5, true));
    return 1;
}
