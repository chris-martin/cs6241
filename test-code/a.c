#include <stdio.h>

int b() {
  return 6;
}

int a(int i) {
  b();
  if (i > 5) {
    return i + 4;
  } else {
    return i + 8;
  }
}

int main() {
  a(8);
	for (int i = 0; i < 10; i++) {
		a(i);
	}
	a(24);
  return 0;
}

