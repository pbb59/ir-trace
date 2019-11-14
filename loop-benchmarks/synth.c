#include <stdio.h>

// double loop nest synthetic benchmark
float synth(float a, float b) {
  if (a > 1.0f) {
    if (b > 1.0f) {
      return a + b;
    }
    else {
      return a * b;
    }
  }
  else {
    if (b > 3.0f) {
      return a - b;
    }
    else {
      return a / b;
    }
  }
}

int main() {
  float ret = synth(3.0f, 0.5f);
  printf("%f\n", ret);
}