#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  L Memo::get(Inequality inequality) {
    Diff diff = inequality.diff();
    APInt c = inequality.c();
    L l = diff.fixedBoundGet(c);
    if (l != L_NULL) return l;
    if (memo.find(diff) == memo.end()) return L_NULL;
    return memo[diff]->get(c);
  }

  void Memo::modify(bool strengthen, Inequality inequality, L l) {
    Diff diff = inequality.diff();
    APInt c = inequality.c();
    LBound *bound;
    if (memo.find(diff) == memo.end()) {
      bound = LBound::create();
      memo[diff] = bound;
    } else {
      bound = memo[diff];
    }
    bound->modify(strengthen, l, c);
    if (bound->isVacuous()) memo.erase(diff);
  }

}
