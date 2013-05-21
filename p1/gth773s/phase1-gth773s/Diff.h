#ifndef LLVM_PHASEONE_DIFF
#define LLVM_PHASEONE_DIFF

using namespace llvm;
using namespace std;

namespace phaseone {

  class Diff {

public:

    static Diff create(Value *a, Value *b);

    inline Value *a() { return _a; };

    inline Value *b() { return _b; };

    bool isConstant();

    L fixedBoundGet(APInt c);

    bool operator<(const Diff &o) const;

private:

    Value *_a;

    Value *_b;

    Integer constantValue();

  };

}

#endif
