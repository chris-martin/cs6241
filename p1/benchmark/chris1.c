void foo() {
  int arr[55];
  arr[44] = 99;
}
void bar(int xyz) {
  int b[xyz];
  b[10] = 20;
  xyz = 41;
  int c[xyz];
  c[40] = 50;
}
int main(int argc, char **argv) {
  bar(11);
}

