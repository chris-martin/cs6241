int main() {
}
struct s {
  int x[10][20];
};
int foo(struct s *p) {
  return p->x[5][15];
}

