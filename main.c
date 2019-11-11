#include <stdio.h>

int func1(int a) {
  return a + 1;
}

int func2(int a) {
  return func1(a) - 1;
}

int test(int a, int b) {
    int c = 0;
    if (a > b) {
      if (a > 3) {
        a = func2(a);
      }
      else {
        a = func1(a);
      }
      c = a - b;
    }
    else {
      a = func1(a);
      c = b - a;
    }
    c+=1;
    return c;
}

int main() {
    printf("%d\n", test(4, 3));
}
