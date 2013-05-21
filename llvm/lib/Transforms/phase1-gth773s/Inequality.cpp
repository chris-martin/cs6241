#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  Inequality Inequality::create(Diff diff, APInt c) {
    Inequality ineq;
    ineq._diff = diff;
    ineq._c = c;
    return ineq;
  }

  Inequality Inequality::create(Value *a, Value *b, APInt c) {
    return Inequality::create(Diff::create(a, b), c);
  }

  bool Inequality::operator<(const Inequality &o) const {
    return _diff < o._diff || _c.sle(o._c);
  }

  void Inequality::print(raw_ostream &o) {
    WriteAsOperand(o, b());
    o << " - ";
    WriteAsOperand(o, a());
    o << " <= " << c();
  }

  void Inequality::printForDot(raw_ostream &o) {
    o << "\"";
    WriteAsOperand(o, a());
    o << "\" -> \"";
    WriteAsOperand(o, b());
    o << "\" [label=\"" << c() << "\"]\n";
  }

  void Inequality::printForJava(raw_ostream &o) {
    o << "add(v(\"";
    WriteAsOperand(o, a());
    o << "\"), v(\"";
    WriteAsOperand(o, b());
    o << "\"), " << c() << ");\n";
  }

  raw_ostream &operator<<(raw_ostream &o, Inequality &i) {
    i.print(o);
    return o;
  }

}
