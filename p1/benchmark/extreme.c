#include <stdio.h>
int main(int argc, char ** argv) {
  int arr[1000000];
  int i, j, k, sum;
  for (k = 0; k < 1000; k++) {
    for (i = 0; i < 1000000; i++) {
      arr[i] = i;
    }
    sum = 0;
    for (j = 999999; j >=0; j--) {
      sum += arr[j];
    }
  }
  printf("%d", sum);
  return 0;
}

