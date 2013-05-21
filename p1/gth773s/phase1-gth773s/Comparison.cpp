#include "PhaseOne.h"

using namespace llvm;

namespace phaseone {

  void Comparison::setValue(int operand, int which, Value *v) {
    if (operand == 0 && which == 0) { _v_i = v; return; }
    if (operand == 0 && which == 1) { _v_j = v; return; }
    if (operand == 0 && which == 2) { _v_k = v; return; }
    if (operand == 1 && which == 0) { _w_r = v; return; }
    if (operand == 1 && which == 1) { _w_s = v; return; }
    if (operand == 1 && which == 2) { _w_t = v; return; }
  }

}
