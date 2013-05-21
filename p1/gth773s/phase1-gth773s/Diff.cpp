#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  Diff Diff::create(Value *a, Value *b) {
    Diff diff;
    diff._a = a;
    diff._b = b;
    return diff;
  }

  bool Diff::isConstant() {
    return (_a == _b) || (isa<ConstantInt>(_a) && isa<ConstantInt>(_b));
  }

  Integer Diff::constantValue() {
    if (_a == _b) return Integer::some(APInt(1, 0));
    if (ConstantInt *cA = dyn_cast<ConstantInt>(_a)) {
      if (ConstantInt *cB = dyn_cast<ConstantInt>(_b)) {
        return Integer::some(cB->getValue() - cA->getValue());
      }
    }
    return Integer::none();
  }

  L Diff::fixedBoundGet(APInt c) {
    Integer cv = constantValue();
    if (!cv.hasValue()) return L_NULL;
    LBound *bound = LBound::fixed(cv.value());
    L l = bound->get(c);
    delete bound;
    return l;
  }

  bool Diff::operator<(const Diff &o) const {
    return (_a != o._a) ? (_a < o._a) : (_b < o._b);
  }

}
