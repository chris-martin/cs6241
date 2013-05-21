#ifndef LLVM_PHASEONE_ABC_REMOVER
#define LLVM_PHASEONE_ABC_REMOVER

using namespace llvm;
using namespace std;

namespace phaseone {

  class AbcRemover {

public:

    static AbcRemover *create(Function *f, DominatorTree *dt, bool debug);

    bool run();

private:

    Function *F;
    DominatorTree *DT;
    bool debug;
    set<Comparison*> comparisons;
    list<Pi*> piNodeList;
    map<PHINode*, Pi*> phiToPi;
    Prover *prover;
    int branchesTested;
    set<TerminatorInst*> branchesToRemove;

    void nameAllValues();
    void nameAllBlocks();

    Pi *addPi(Pi *pi);

    Pi *addPi(Value *value, BasicBlock *from, BasicBlock *to, bool taken);

    Value *piOrConstant(Value *v, bool taken);

    void findComparisons();

    void insertPiNodes();

    void removePiNodes();

    //Pi *getPi(Value *v, bool taken);

    void findICmpInequalities(Comparison *c);

    void findInequalities();

    void demandProve();

    void removeBranches();

  };

}

#endif
