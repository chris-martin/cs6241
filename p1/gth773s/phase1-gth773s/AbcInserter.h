#ifndef LLVM_PHASEONE_ABC_INSERTER
#define LLVM_PHASEONE_ABC_INSERTER

using namespace llvm;
using namespace std;

namespace phaseone {

  class AbcInserter {

public:

    static AbcInserter *create(Function *f);

    bool run();

private:

    Function *function;

    // Insert a call to exit(1) before i.
    // See ChangeToUnreachable in SimplifyCFGPass.cpp for similar usage.
    // Returns the new trap call instruction.
    CallInst *insertExitCall(Instruction *i);

    // Inserts a branch on cmp and a trap call before instruction i.
    // The branch leads to i if cmp evaluates true, or to the trap call otherwise.
    // Returns the new branch instruction.
    void insertAssertion(Instruction *i, CmpInst *cmp);

    // Insert "0 <= index < bound" assertion before i.
    // Returns the new branch instruction.
    void insertBoundCheck(Instruction *i, Value *lower, Value *v, Value *upper);

    // get the instruction where the dynamic array length is defined,
    // ie. %17 = mul i64 %16, 4
    Instruction *findDynArrayLength(GetElementPtrInst *gep);

    bool splitGeps();

  };

}

#endif
