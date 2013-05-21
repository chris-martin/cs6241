#ifndef LLVM_PHASEONE_INEQUALITY
#define LLVM_PHASEONE_INEQUALITY

namespace phaseone {

  // b - a <= c
  class Inequality {

public:

    static Inequality create(Diff diff, APInt c);

    static Inequality create(Value *a, Value *b, APInt c);

    inline Diff diff() { return _diff; }

    inline Value *a() { return _diff.a(); }

    inline Value *b() { return _diff.b(); }

    inline APInt c() { return _c; }

    bool operator<(const Inequality &o) const;

    void print(raw_ostream &o);

    void printForDot(raw_ostream &o);

    void printForJava(raw_ostream &o);

private:

    Diff _diff;

    APInt _c;

  };

  raw_ostream &operator<<(raw_ostream &o, Inequality &i);

}

#endif
