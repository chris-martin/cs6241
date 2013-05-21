#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  void LBound::clear() {
    iFalse = iReduced = iTrue = Integer::none();
  }

  LBound *LBound::create() {
    LBound *bound = new LBound();
    bound->clear();
    return bound;
  }

  LBound *LBound::fixed(APInt c) {
    LBound *bound = new LBound();
    bound->iTrue = Integer::some(c);
    bound->iFalse = Integer::some(c - APInt(1, 1));
    bound->iReduced = Integer::none();
    return bound;
  }

  L LBound::get(APInt c) {
    if (iTrue.hasValue() && iTrue.value().sle(c)) return L_TRUE;
    if (iFalse.hasValue() && iFalse.value().sge(c)) return L_FALSE;
    if (iReduced.hasValue() && iReduced.value().sle(c)) return L_REDUCED;
    return L_NULL;
  }

  bool LBound::isVacuous() {
    return !(iFalse.hasValue() || iReduced.hasValue() || iTrue.hasValue());
  }

  Integer LBound::get(L l) {
    switch (l) {
      case L_TRUE: return iTrue;
      case L_REDUCED: return iReduced;
      case L_FALSE: return iFalse;
      default: return Integer::none();
    }
  }

  void LBound::put(L l, Integer c) {
    switch (l) {
      case L_TRUE: iTrue = c; break;
      case L_REDUCED: iReduced = c; break;
      case L_FALSE: iFalse = c; break;
      default: errs() << "FAIL\n";
    }
  }

  bool LBound::isMaxBound(L l) {
    return l == L_FALSE;
  }

  APInt LBound::max(APInt a, APInt b) {
    return a.slt(b) ? b : a;
  }

  APInt LBound::min(APInt a, APInt b) {
    return a.sgt(b) ? b : a;
  }

  Integer LBound::strongerBound(L l, Integer a, APInt b) {
    if (!a.hasValue()) return Integer::some(b);
    Integer result = Integer::some(isMaxBound(l) ? max(a.value(), b) : min(a.value(), b));
    //errs() << "Strengthen " << l << ", " << a << ", " << b << ": " << result << "\n";
    return result;
  }

  Integer LBound::weakerBound(L l, Integer a, APInt b) {
    if (!a.hasValue()) return Integer::none();
    Integer result = Integer::some(isMaxBound(l) ? min(a.value(), b) : max(a.value(), b));
    //errs() << "Weaken " << l << ", " << a << ", " << b << ": " << result << "\n";
    return result;
  }

  void LBound::modify(bool strengthen, L l, APInt c) {
    if (strengthen) {
      if (l != L_NULL) {
        Integer currentBound = get(l);
        Integer newBound = strongerBound(l, currentBound, c);
        put(l, newBound);
      }
    } else {
      if (l == L_NULL) {
        clear();
      } else {
        Integer currentBound = get(l);
        Integer newBound = weakerBound(l, get(l), c);
        put(l, newBound);
      }
    }
  }

  raw_ostream &operator<<(raw_ostream &o, L &i) {
    if (i == L_FALSE) {
      o << "false";
    } else if (i == L_TRUE) {
      o << "true";
    } else if (i == L_REDUCED) {
      o << "reduced";
    } else {
      o << "null";
    }
    return o;
  }

}
