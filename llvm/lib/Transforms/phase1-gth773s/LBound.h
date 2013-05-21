#ifndef LLVM_PHASEONE_LBOUND
#define LLVM_PHASEONE_LBOUND

using namespace llvm;
using namespace std;

namespace phaseone {

  enum L {
    L_TRUE,
    L_REDUCED,
    L_FALSE,
    L_NULL
  };

  class LBound {

public:

    static LBound *create();

    static LBound *fixed(APInt c);

    L get(APInt c);

    void modify(bool strengthen, L l, APInt c);

    bool isVacuous();

private:

    Integer iFalse, iReduced, iTrue;

    void clear();

    Integer get(L l);

    void put(L l, Integer c);

    static bool isMaxBound(L l);

    APInt max(APInt a, APInt b);

    APInt min(APInt a, APInt b);

    Integer strongerBound(L l, Integer a, APInt b);

    Integer weakerBound(L l, Integer a, APInt b);

  };

  raw_ostream &operator<<(raw_ostream &o, L &i);

}

#endif
