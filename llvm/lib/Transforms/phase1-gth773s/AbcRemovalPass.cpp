#include "PhaseOne.h"

namespace phaseone {

  struct AbcRemoval :	public llvm::FunctionPass {

    static char ID;

    bool debug;

    AbcRemoval() : llvm::FunctionPass(ID) { debug = true; }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
      AU.addRequiredID(BreakCriticalEdgesID);
      AU.addRequired<DominatorTree>();
    }

    virtual bool runOnFunction(llvm::Function &f) {
      AbcRemover *x = AbcRemover::create(&f, &getAnalysis<llvm::DominatorTree>(), debug);
      bool modified = x->run();
      delete x;
      return modified;
    }

  };

  char AbcRemoval::ID = 2;
  INITIALIZE_PASS(AbcRemoval, "abc-remove", "CS 6241 Abc Removal", false, false);

}

namespace llvm {

  FunctionPass *createABCDPass() {
    phaseone::AbcRemoval *x = new phaseone::AbcRemoval();
    x->debug = false;
    return x;
  }

}

