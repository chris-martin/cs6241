#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  Integer Integer::none() {
    Integer i;
    i._hasValue = false;
    return i;
  }

  Integer Integer::some(APInt v) {
    Integer i;
    i._hasValue = true;
    i._value = v;
    return i;
  }

  void Integer::print(raw_ostream &o) {
    if (hasValue()) {
      o << value();
    } else {
      o << "nil";
    }
  }

  raw_ostream &operator<<(raw_ostream &o, Integer &i) {
    i.print(o);
    return o;
  }

}
