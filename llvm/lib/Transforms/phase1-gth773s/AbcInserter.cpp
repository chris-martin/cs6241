#include "PhaseOne.h"

using namespace llvm;
using namespace std;

namespace phaseone {

  AbcInserter *AbcInserter::create(Function *f) {
    AbcInserter *inserter = new AbcInserter();
    inserter->function = f;
    return inserter;
  }

  bool AbcInserter::run() {

    bool modifiedAnything = splitGeps();
    unsigned int checksInserted = 0;

    set<GetElementPtrInst*> geps;
    for (inst_iterator i = inst_begin(function), e = inst_end(function); i != e; ++i)
      if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(&*i))
        geps.insert(&*gep);

    for (set<GetElementPtrInst*>::iterator gep_i = geps.begin(),
        gep_e = geps.end(); gep_i != gep_e; ++gep_i) {
      GetElementPtrInst *gep = *gep_i;
      gep_type_iterator gti = ++gep_type_begin(gep);
      if (const ArrayType *at = dyn_cast<ArrayType>(*gti)) {
        Value *v = gep->getOperand(gep->getNumOperands() - 1);
        const Type *t = cast<IntegerType>(v->getType());
        Value *lower = ConstantInt::get(t, 0);
        Value *upper = ConstantInt::get(t, at->getNumElements());
        insertBoundCheck(gep, lower, v, upper);
        modifiedAnything = true; ++checksInserted;
      } else if (dyn_cast<IntegerType>(*gti)) {
        Value *v = gep->getOperand(gep->getNumOperands() - 1);
        if (const Type *t = dyn_cast<IntegerType>(v->getType())) {
          Value *lower = ConstantInt::get(t, 0);
          Value *upper = findDynArrayLength(gep);
          if (upper == 0 || upper->getType() != t) continue;
          insertBoundCheck(gep, lower, v, upper);
          modifiedAnything = true; ++checksInserted;
        }
      }
    }

    errs() << " - " << function->getName() << ": inserted " << checksInserted << " checks\n";

    return modifiedAnything;

  }

  // this is mostly copied from GEPSplitter.cpp
  bool AbcInserter::splitGeps() {
    bool Changed = false;

    // Visit each GEP instruction.
    for (Function::iterator I = function->begin(), E = function->end(); I != E; ++I) {
      for (BasicBlock::iterator II = I->begin(), IE = I->end(); II != IE; ) {
        if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(II++)) {
          unsigned NumOps = GEP->getNumOperands();
          // Ignore GEPs which are already simple.
          if (NumOps <= 2) continue;

          bool FirstIndexIsZero = isa<ConstantInt>(GEP->getOperand(1)) &&
                                  cast<ConstantInt>(GEP->getOperand(1))->isZero();
          if (NumOps == 3 && FirstIndexIsZero)
            continue;

          // The first index is special and gets expanded with a 2-operand GEP
          // (unless it's zero, in which case we can skip this).
          Value *NewGEP = FirstIndexIsZero ?
            GEP->getOperand(0) :
            GetElementPtrInst::Create(GEP->getOperand(0), GEP->getOperand(1),
                                      "tmp", GEP);
          // All remaining indices get expanded with a 3-operand GEP with zero
          // as the second operand.
          Value *Idxs[2];
          Idxs[0] = ConstantInt::get(Type::getInt64Ty(function->getContext()), 0);
          for (unsigned i = 2; i != NumOps; ++i) {
            Idxs[1] = GEP->getOperand(i);
            NewGEP = GetElementPtrInst::Create(NewGEP, Idxs, Idxs+2, "tmp", GEP);
          }
          GEP->replaceAllUsesWith(NewGEP);
          GEP->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed;
  }

  Instruction *AbcInserter::findDynArrayLength(GetElementPtrInst *gep) {
    Value *gepOperand = gep->getOperand(0);
    if (BitCastInst *bitcast = dyn_cast<BitCastInst>(gepOperand)) {
      if (bitcast->getNumOperands() == 0) return 0;
      Value *bitcastOperand = bitcast->getOperand(0);
      if (AllocaInst *alloca = dyn_cast<AllocaInst>(bitcastOperand)) {
        if (alloca->getNumOperands() == 0) return 0;
        Value *allocaOperand = alloca->getOperand(0);
        if (BinaryOperator *bin = dyn_cast<BinaryOperator>(allocaOperand)) {
          if (bin->getNumOperands() < 2) return 0;
          return dyn_cast<Instruction>(bin->getOperand(1));
        }
      }
    }
    return 0;
  }

  // Insert "0 <= index < bound" assertion before i.
  void AbcInserter::insertBoundCheck(Instruction *i, Value *lower, Value *v, Value *upper) {
    ICmpInst *cmp1 = new ICmpInst(ICmpInst::ICMP_SLT, v, upper);
    ICmpInst *cmp2 = new ICmpInst(ICmpInst::ICMP_SLE, lower, v);
    insertAssertion(i, cmp1);
    insertAssertion(cmp1, cmp2);
  }

  // Inserts a branch on cmp and a trap call before instruction i.
  // The branch leads to i if cmp evaluates true, or to the trap call otherwise.
  // Returns the new comparison instruction.
  void AbcInserter::insertAssertion(Instruction *i, CmpInst *cmp) {

    cmp->insertBefore(i);
    CallInst *exitInst = insertExitCall(i);

    // instruction list
    //   cmp
    //   exit
    //   i

    BasicBlock *block1 = i->getParent();
    BasicBlock *exitBlock = block1->splitBasicBlock(exitInst);
    BasicBlock *block2 = exitBlock->splitBasicBlock(i);

    // cfg
    //   block1 -> exitBlock
    //   exitBlock -> block2
    //   block2 -> ...

    block2->removePredecessor(exitBlock);
    TerminatorInst *exitTerminator = exitBlock->getTerminator();
    new UnreachableInst(exitTerminator->getContext(), exitTerminator);
    exitTerminator->eraseFromParent();

    // cfg
    //   block1 -> exitBlock
    //   block2 -> ...

    BranchInst *br = BranchInst::Create(block2, exitBlock, cmp);
    ReplaceInstWithInst(block1->getTerminator(), br);

    // cfg
    //   block1 -> exitBlock
    //   block1 -> block2
    //   block2 -> ...

  }

  // Insert a call to exit(1) before i.
  // See ChangeToUnreachable in SimplifyCFGPass.cpp for similar usage.
  // Returns the new trap call instruction.
  CallInst *AbcInserter::insertExitCall(Instruction *i) {
    Function *f = i->getParent()->getParent();
    Module *m = f->getParent();
    const Type *voidType = Type::getVoidTy(m->getContext());
    const Type *intType = IntegerType::get(m->getContext(), 32);
    Constant *exitCall = m->getOrInsertFunction("exit", voidType, intType, NULL);
    Value *exitCode = ConstantInt::get(Type::getInt32Ty(f->getContext()), 1);
    return CallInst::Create(exitCall, exitCode, "", i);
  }

}
