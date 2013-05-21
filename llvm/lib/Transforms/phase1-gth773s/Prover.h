#ifndef LLVM_PHASEONE_PROVER
#define LLVM_PHASEONE_PROVER

using namespace llvm;
using namespace std;

namespace phaseone {

  class Prover {

public:

    void add(Inequality edge);

    void add(Value *u, Value *v, APInt c);

    void finish();

    L prove(Inequality inequality);

    bool demandProve(Inequality inequality);

    void printGraph();

private:

    void addConstant(Value *v);

    set<ConstantInt*> constants;

    multimap<Value*, Inequality> incomingEdges;

    map<Diff, APInt> edges;

    void addEdge(Inequality i);

    map<Value*, APInt> active;

    Memo memo;

  };

}

#endif
