#ifndef LLVM_PHASEONE_MEMO
#define LLVM_PHASEONE_MEMO

using namespace llvm;
using namespace std;

namespace phaseone {

  class Memo {

public:

    L get(Inequality inequality);

    void modify(bool strengthen, Inequality inequality, L l);

private:

    map<Diff, LBound*> memo;

  };

}

#endif
