#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  void Prover::addConstant(Value *v) {
    if (ConstantInt *c = dyn_cast<ConstantInt>(v)) {
      constants.insert(c);
    }
  }

  void Prover::add(Inequality edge) {
    if (edge.diff().isConstant()) return;
    addEdge(edge);
    addConstant(edge.diff().a());
    addConstant(edge.diff().b());
  }

  void Prover::add(Value *u, Value *v, APInt c) {
    add(Inequality::create(u, v, c));
  }

  void Prover::addEdge(Inequality edge) {
    if (edges.find(edge.diff()) == edges.end()) {
      incomingEdges.insert(pair<Value*, Inequality>(edge.b(), edge));
      edges.insert(pair<Diff, APInt>(edge.diff(), edge.c()));
    } else {
      APInt oldC = edges[edge.diff()];
      if (oldC.sgt(edge.c())) {
        edges.insert(pair<Diff, APInt>(edge.diff(), edge.c()));
      }
    }
  }

  void Prover::finish() {
  }

  L Prover::prove(Inequality inequality) {

    Value *a = inequality.a();
    Value *b = inequality.b();
    APInt c = inequality.c();

    L alreadyProven = memo.get(inequality);
    if (alreadyProven != L_NULL) return alreadyProven;
    if (incomingEdges.find(b) == incomingEdges.end()) return L_FALSE;
    if (active.find(b) != active.end()) return c.sgt(active[b]) ? L_FALSE : L_REDUCED;
    active[b] = c;

    bool isPhiNode = false;
    if (PHINode *phi = dyn_cast<PHINode>(b)) {
      if (phi->getNumIncomingValues() > 1) isPhiNode = true;
    }
    bool strengthen = !isPhiNode;

    for (multimap<Value*, Inequality>::iterator i = incomingEdges.lower_bound(b),
         e = incomingEdges.upper_bound(b); i != e; ++i) {
      Inequality pred = (*i).second;
      L subResult = prove(Inequality::create(a, pred.a(), c - pred.c()));
      memo.modify(strengthen, inequality, subResult);
    }

    active.erase(b);

    L result = memo.get(inequality);
    //errs() << "Tried to prove " << inequality << " - got " << result << "!";
    //if (isPhiNode) { errs() << " (PHI NODE)"; }
    //errs() << "\n";
    return result;

  }

  bool Prover::demandProve(Inequality inequality) {
    active.clear();
    L l = prove(inequality);
    bool proven = l == L_TRUE || l == L_REDUCED;
    //errs() << "Result for " << inequality << ": " << proven << "\n";
    return proven;
  }

  void Prover::printGraph() {
    errs() << "digraph f {\n";
    for (multimap<Value*, Inequality>::iterator i = incomingEdges.begin(),
            e = incomingEdges.end(); i != e; ++i) {
      Inequality edge = (*i).second;
      edge.printForDot(errs());
    }
    errs() << "}\n";
  }

}
