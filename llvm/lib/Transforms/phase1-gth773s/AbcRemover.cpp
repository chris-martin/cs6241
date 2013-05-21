#include "PhaseOne.h"
#include <sstream>

using namespace llvm;
using namespace std;

namespace phaseone {

  AbcRemover *AbcRemover::create(Function *f, DominatorTree *dt, bool debug) {
    AbcRemover *remover = new AbcRemover();
    remover->F = f;
    remover->DT = dt;
    remover->debug = debug;
    return remover;
  }

  void AbcRemover::nameAllValues() {
    unsigned nextName = 1;
    for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {
      if (!(&*i)->hasName() && !(&*i)->getType()->isVoidTy()) {
        std::stringstream x;
        x << "i" << nextName++;
        (&*i)->setName(x.str());
      }
    }
  }

  void AbcRemover::nameAllBlocks() {
    unsigned nextName = 1;
    for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
      BasicBlock *b = &*i;
      if (!b->hasName()) {
        std::stringstream x;
        x << "b" << nextName++;
        b->setName(x.str());
      }
    }
  }

  bool AbcRemover::run() {
    branchesTested = 0;
    prover = new Prover();
    findComparisons();
    nameAllValues();
    nameAllBlocks();
    insertPiNodes();
    findInequalities();
    prover->finish();
    //prover->printGraph();
    demandProve();
    delete prover;
    int branchesRemoved = branchesToRemove.size();
    removePiNodes();
    removeBranches();
    if (debug) {
      errs() << " - " << F->getName() << ": inserted " << piNodeList.size() << " pis";
      errs() << ", tested " << branchesTested << " branches";
      errs() << ", removed " << branchesRemoved << " branches\n";
    }
    return !piNodeList.empty();
  }

  void AbcRemover::findComparisons() {
    for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) {
      TerminatorInst *t = (&*B)->getTerminator();
      if (t == 0 || t->getNumOperands() == 0) continue;
      if (t->getNumSuccessors() != 2) continue;
      if (ICmpInst *icmp = dyn_cast<ICmpInst>(t->getOperand(0))) {
        switch (icmp->getPredicate()) {
          case CmpInst::ICMP_SGT: case CmpInst::ICMP_UGT:
          case CmpInst::ICMP_SGE: case CmpInst::ICMP_UGE:
          case CmpInst::ICMP_SLT: case CmpInst::ICMP_ULT:
          case CmpInst::ICMP_SLE: case CmpInst::ICMP_ULE:
            break;
          default:
            continue;
        }
        if (icmp->getOperand(0) == icmp->getOperand(1)) continue;
        Comparison *c = new Comparison();
        c->terminator(t);
        c->icmp(icmp);
        comparisons.insert(c);
      }
    }
  }

  void AbcRemover::insertPiNodes() {

    // insert pi nodes
    for (set<Comparison*>::iterator c = comparisons.begin(), E = comparisons.end(); c != E; ++c) {
      BasicBlock *from = (*c)->terminator()->getParent();
      for (int i = 0; i < 2; ++i) {
        Value *v = (*c)->icmp()->getOperand(i);
        (*c)->setValue(i, 0, v);
        for (int s = 0; s < 2; ++s) {
          Value *branchValue = 0;
          if (isa<ConstantInt>(v)) {
            branchValue = v;
          } else {
            BasicBlock *to = (*c)->terminator()->getSuccessor(s);
            bool taken = s == 0;
            Pi *pi = addPi(v, from, to, taken);
            branchValue = pi->phi();
          }
          (*c)->setValue(i, s + 1, branchValue);
        }
      }
    }

    // substitute values with pi values
    for (list<Pi*>::iterator pi = piNodeList.begin(), pi_e = piNodeList.end(); pi != pi_e; ++pi) {
      Value *v = (*pi)->value();
      PHINode *phi = (PHINode*) (*pi)->phi();
      //errs() << "Trying to sub usages of " << v->getNameStr() << " with " << phi->getNameStr() << "\n";
      //errs() << "There are " << v->getNumUses() << " usages to consider.\n";
      set<Value*> uses;
      for (Value::use_iterator use = v->use_begin(), use_e = v->use_end(); use != use_e; ++use) {
        uses.insert(*use);
      }
      for (set<Value*>::iterator use_i = uses.begin(), use_e = uses.end(); use_i != use_e; ++use_i) {
        Value *use = *use_i;
        //errs() << "  Usage:";
        //(use)->print(errs());
        //errs() << "\n";
        if (Instruction *useInst = dyn_cast<Instruction>(use)) {
          if (useInst != phi) {
            bool dominates = DT->dominates(phi, useInst);
            //errs() << "    Dominates: " << (dominates ? "yes" : "no") << "\n";
            if (dominates) {
              useInst->replaceUsesOfWith(v, phi);
              if (PHINode *phiUse = dyn_cast<PHINode>(useInst)) {
                if (phiToPi.find(phiUse) != phiToPi.end()) {
                  Pi *piUse = phiToPi[phiUse];
                  piUse->updatePhiName();
                }
              }
            }
          }
        }
      }
    }

  }

  Pi *AbcRemover::addPi(Pi *pi) {
    piNodeList.push_back(pi);
    phiToPi.insert(pair<PHINode*, Pi*>(pi->phi(), pi));
    return pi;
  }

  Pi *AbcRemover::addPi(Value *value, BasicBlock *from, BasicBlock *to, bool taken) {
    return addPi(Pi::create(value, from, to, taken));
  }

  void AbcRemover::findInequalities() {
    for (set<Comparison*>::iterator c = comparisons.begin(), E = comparisons.end(); c != E; ++c) {
      findICmpInequalities(*c);
    }
  }

  void AbcRemover::findICmpInequalities(Comparison *c) {

    APInt zero = APInt(1, 0);
    APInt one = APInt(1, 1);
    APInt minusone = APInt(1, -1, true);

    switch (c->icmp()->getPredicate()) {
      case CmpInst::ICMP_SGT: case CmpInst::ICMP_UGT:
        break;
      case CmpInst::ICMP_SGE: case CmpInst::ICMP_UGE:
        break;
      case CmpInst::ICMP_SLT: case CmpInst::ICMP_ULT:
        prover->add(c->v_i(), c->v_j(), zero);
        prover->add(c->w_r(), c->w_s(), zero);
        prover->add(c->w_s(), c->v_j(), minusone);
        prover->add(c->v_i(), c->v_k(), zero);
        prover->add(c->w_r(), c->w_t(), zero);
        prover->add(c->v_k(), c->w_t(), zero);
        break;
      case CmpInst::ICMP_SLE: case CmpInst::ICMP_ULE:
        prover->add(c->v_i(), c->v_j(), zero);
        prover->add(c->w_r(), c->w_s(), zero);
        prover->add(c->w_s(), c->v_j(), zero);
        prover->add(c->v_i(), c->v_k(), zero);
        prover->add(c->w_r(), c->w_t(), zero);
        prover->add(c->v_k(), c->w_t(), minusone);
        break;
      default:
        return;
    }

  }

  void AbcRemover::removePiNodes() {
    for (list<Pi*>::reverse_iterator pi = piNodeList.rbegin(), E = piNodeList.rend(); pi != E; ++pi) {
      Value *v = (*pi)->value();
      PHINode *phi = (PHINode*) (*pi)->phi();
      phi->replaceAllUsesWith(v);
      phi->eraseFromParent();
    }
  }

  void AbcRemover::demandProve() {
    for (set<Comparison*>::iterator c_i = comparisons.begin(), c_e = comparisons.end(); c_i != c_e; ++c_i) {
      Comparison *c = *c_i;
      BasicBlock *killTarget = cast<BasicBlock>(c->terminator()->getOperand(1));
      // all of our bounds checks have a false branch that leads to a block with no successors
      if (killTarget->getTerminator()->getNumSuccessors() == 0) {
        branchesTested++;
        Value *x = c->icmp()->getOperand(0);
        Value *y = c->icmp()->getOperand(1);
        Inequality inequality;
        switch (c->icmp()->getPredicate()) {
          case CmpInst::ICMP_SGT: case CmpInst::ICMP_UGT:
            inequality = Inequality::create(x, y, APInt(1, -1, true)); break;
          case CmpInst::ICMP_SGE: case CmpInst::ICMP_UGE:
            inequality = Inequality::create(x, y, APInt(1, 0)); break;
          case CmpInst::ICMP_SLT: case CmpInst::ICMP_ULT:
            inequality = Inequality::create(y, x, APInt(1, -1, true)); break;
          case CmpInst::ICMP_SLE: case CmpInst::ICMP_ULE:
            inequality = Inequality::create(y, x, APInt(1, 0)); break;
          default:
            continue;
        }
        if (prover->demandProve(inequality)) {
          branchesToRemove.insert(c->terminator());
        }
      }
    }
  }

  void AbcRemover::removeBranches() {
    for (set<TerminatorInst*>::iterator t = branchesToRemove.begin(), e = branchesToRemove.end(); t != e; ++t) {
      BasicBlock *killTarget = cast<BasicBlock>((*t)->getOperand(1));
      BasicBlock *branchTarget = cast<BasicBlock>((*t)->getOperand(2));
      killTarget->removePredecessor((*t)->getParent());
      BranchInst *unconditionalBranch = BranchInst::Create(branchTarget);
      ICmpInst *icmp = cast<ICmpInst>((*t)->getOperand(0));
      ReplaceInstWithInst(*t, unconditionalBranch);
      icmp->eraseFromParent();
      killTarget->eraseFromParent();
    }
  }

}
