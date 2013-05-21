#include "PhaseOne.h"

namespace phaseone {

  struct AbcInsertion : public llvm::FunctionPass {

    static char ID;

    AbcInsertion() : llvm::FunctionPass(ID) { }

    virtual bool runOnFunction(llvm::Function &f) {
      AbcInserter *inserter = AbcInserter::create(&f);
      bool modified = inserter->run();
      delete inserter;
      return modified;
    }

  };

  char AbcInsertion::ID = 1;
  INITIALIZE_PASS(AbcInsertion, "abc-insert", "CS 6241 Abc Insertion", false, false);

}
