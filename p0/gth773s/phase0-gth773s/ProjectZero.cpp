#define DEBUG_TYPE "projectzero"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Instructions.h"
#include <limits>
using namespace llvm;
using namespace std;

namespace {
  
  // data structure that aggregrates information about a collection
  // of ints which are provided one-at-a-time using the add method
  struct IntStats {
    // the lowest int in the collection
    int min;
    // the highest int in the collection
    int max;
    // the sum of all ints in the collection
    int total;
    // the size of the collection
    int size;
    // constructor, initializes everything to 0
    IntStats() {
      max = total = size = 0;
      min = std::numeric_limits<int>::max();
    }
    // adds an int to the collection
    void add(int a) {
      if (a < min) min = a;
      if (a > max) max = a;
      total += a;
      ++size;
    }
    // adds everything from another IntStats to the collection
    void add(IntStats *is) {
      if (is->min < min) min = is->min;
      if (is->max > max) max = is->max;
      total += is->total;
      size += is->size;
    }
    // prints a human-readable string to the given stream
    void print(raw_ostream &O) {
      if (size) { O << "min " << min << ", max " << max
        << ", total " << total << ", average " << ((double) total / size); }
      else { O << "no data"; }
    }
  };
  
  // override << so IntStats can be concatenated to a raw_ostream
  raw_ostream &operator<<(raw_ostream &o, IntStats &mmt) {
    mmt.print(o);
    return o;
  }
  
  // statistics gathered about a BasicBlock
  struct BasicBlockStats {
    int successors;
    int isLoopBlock;
    int domTreeDescendents;
    int domTreeAncestors;
    // constructor, initializes everything to 0
    BasicBlockStats() {
      successors = isLoopBlock = domTreeDescendents = domTreeAncestors = 0;
    }
  };
  
  // statistics gathered about a Function
  struct FunctionStats {
    int functionCalls;
    int basicBlocks;
    int cfgEdges;
    int loopBlocks;
    int singleEntryLoops;
    int domTreeDescendents;
    int domTreeAncestors;
    // constructor, initializes everything to 0
    FunctionStats() {
      functionCalls = basicBlocks = cfgEdges = loopBlocks = singleEntryLoops = 0;
      domTreeDescendents = domTreeAncestors = 0;
    }
    void add(BasicBlockStats *bStats) {
      ++basicBlocks;
      loopBlocks += bStats->isLoopBlock;
      cfgEdges += bStats->successors;
      domTreeDescendents += bStats->domTreeDescendents;
      domTreeAncestors += bStats->domTreeAncestors;
    }
  };
  
  // statistics gathered about a Module
  struct ModuleStats {
    int functions;
    int functionCalls;
    IntStats* basicBlocks;
    IntStats* cfgEdges;
    IntStats* loopBlocks;
    IntStats* singleEntryLoops;
    int domTreeDescendents;
    int domTreeAncestors;
    // constructor
    ModuleStats() {
      functions = functionCalls = 0;
      basicBlocks = new IntStats();
      cfgEdges = new IntStats();
      loopBlocks = new IntStats();
      singleEntryLoops = new IntStats();
      domTreeDescendents = domTreeAncestors = 0;
    }
    // adds the statistics from one function to this aggregate
    void add(FunctionStats *fStats) {
      ++functions;
      functionCalls += fStats->functionCalls;
      basicBlocks->add(fStats->basicBlocks);
      cfgEdges->add(fStats->cfgEdges);
      loopBlocks->add(fStats->loopBlocks);
      singleEntryLoops->add(fStats->singleEntryLoops);
      domTreeDescendents += fStats->domTreeDescendents;
      domTreeAncestors += fStats->domTreeAncestors;
    }
    // prints a human-readable string to the given stream
    void print(raw_ostream &O) {
      O << " (1) Total number of functions in the application: " << functions
      << "\n     Total number of edges in the static call graph: " << functionCalls
      << "\n (2) Basic blocks: " << *basicBlocks
      << "\n (3) CFG edges: " << *cfgEdges
      << "\n (4) Single entry loops: " << *singleEntryLoops
      << "\n (5) Loop basic blocks: " << *loopBlocks
      << "\n (6) Dominator blocks: " << domTreeAncestors
             << ", average per block " << (double) domTreeAncestors / (double) basicBlocks->total
      << "\n (7) Dominated blocks: " << domTreeDescendents
             << ", average per block " << (double) domTreeDescendents / (double) basicBlocks->total
      << "\n";
    }
  };
  
  // override << so ModuleStats can be concatenated to a raw_ostream
  raw_ostream &operator<<(raw_ostream &o, ModuleStats &mStats) {
    mStats.print(o);
    return o;
  }
  
  // The module pass that does the analysis and optimization
  struct ProjectZero : public ModulePass {

    // transpose[A] contains B iff original[B] contains A
    map<BasicBlock*, set<BasicBlock*>*> *transpose(map<BasicBlock*, set<BasicBlock*>*> *original, Function *F) {
      map<BasicBlock*, set<BasicBlock*>*> *transpose = new map<BasicBlock*, set<BasicBlock*>*>();
      for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
        set<BasicBlock*> *s = new set<BasicBlock*>();
        transpose->insert(pair<BasicBlock*, set<BasicBlock*>*>(I, s));
      }
      for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
        BasicBlock *a = I;
        set<BasicBlock*> *s = (*original)[a];
        for (set<BasicBlock*>::iterator J = s->begin(); J != s->end(); ++J) {
          BasicBlock *b = *J;
          (*transpose)[b]->insert(a);
        }
      }
      return transpose;
    }

    // cfg[A] contains B iff (A, B) is an edge in the cfg
    map<BasicBlock*, set<BasicBlock*>*> *getCfg(Function *F) {
      // start with an empty map
      map<BasicBlock*, set<BasicBlock*>*>* cfg = new map<BasicBlock*, set<BasicBlock*>*>();
      // iterate over all blocks I
      for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
        // empty successors set
        set<BasicBlock*>* successorsOfI = new set<BasicBlock*>();
        // add successors set to the map
        cfg->insert(pair<BasicBlock*, set<BasicBlock*>*>(I, successorsOfI));
        // get the block's terminator so we can get its successors
        TerminatorInst *T = I->getTerminator();
        // add each successor to the set
        for (int i = 0; i < T->getNumSuccessors(); i++) successorsOfI->insert(T->getSuccessor(i));
      }
      return cfg;
    }

    // reach[A] contains B iff there is a CFG path from A to B
    map<BasicBlock*, set<BasicBlock*>*> *getReach(Function *F) {
      // Start with the cfg
      map<BasicBlock*, set<BasicBlock*>*>* reach = getCfg(F);
      // Each block is also reachable from itself
      for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) (*reach)[I]->insert(I);
      // Iterate until the map stops changing
      bool notDoneYet = true;
      while (notDoneYet) {
        notDoneYet = false;
        // I : each block
        for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
          set<BasicBlock*>* reachedFromI = (*reach)[I];
          set<BasicBlock*> copy; for (set<BasicBlock*>::iterator J = reachedFromI->begin(); J != reachedFromI->end(); ++J) copy.insert(*J);
          // J : each block reachable from I
          for (set<BasicBlock*>::iterator J = copy.begin(); J != copy.end(); ++J) {
            set<BasicBlock*>* reachedFromJ = (*reach)[*J];
            set<BasicBlock*> copy2; for (set<BasicBlock*>::iterator K = reachedFromJ->begin(); K != reachedFromJ->end(); ++K) copy2.insert(*K);
            // K : each block reachable from J
            for (set<BasicBlock*>::iterator K = copy2.begin(); K != copy2.end(); ++K) {
              // there is a path from I to J to K
              if (reachedFromI->find(*K) == reachedFromI->end()) {
                reachedFromI->insert(*K);
                notDoneYet = true;
              }
            }
          }
        }
      }
      return reach;
    }

    // dom[A] contains B iff A is dominated by B
    map<BasicBlock*, set<BasicBlock*>*> *getDom(Function *F) {
      map<BasicBlock*, set<BasicBlock*>*> *dom = new map<BasicBlock*, set<BasicBlock*>*>();
      map<BasicBlock*, set<BasicBlock*>*> *reverseCfg = transpose(getCfg(F), F);
      for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) {
        set<BasicBlock*> *bDom = new set<BasicBlock*>();
        dom->insert(pair<BasicBlock*, set<BasicBlock*>*>(B, bDom));
        // The entry block is only dominated by itself
        if (B == F->getEntryBlock()) { 
          bDom->insert(B);
        // For all other blocks, start out with all nodes as dominators
        } else {
          for (Function::iterator B2 = F->begin(), E2 = F->end(); B2 != E2; ++B2) bDom->insert(B2);
        }
      }
      bool notDoneYet = true;
      while (notDoneYet) {
        notDoneYet = false;
        // B : Loop over all blocks except the entry block
        for (Function::iterator B = F->begin(), E = F->end(); B != E; ++B) if (B != F->getEntryBlock()) {
          // the predecessors of B
          set<BasicBlock*> *preds = (*reverseCfg)[B];
          // the dominators of B
          set<BasicBlock*> *bDom = (*dom)[B];
          int a = bDom->size();
          // P : Loop over the predecessors of B
          for (set<BasicBlock*>::iterator P = preds->begin(); P != preds->end(); ++P) {
            // the dominators of P
            set<BasicBlock*>* pDom = (*dom)[*P];
            for (Function::iterator B2 = F->begin(), E2 = F->end(); B2 != E2; ++B2) {
              // If B2 doesn't dominate P, then it doesn't dominate B
              if (pDom->find(B2) == pDom->end()) bDom->erase(B2);
            }
            // B dominates itself
            bDom->insert(B);
          }
          int b = bDom->size();
          if (a != b) notDoneYet = true;
        }
      }
      return dom;
    }

    // Pass identification, replacement for typeid
    static char ID;
    
    // Statistics gathered before and after the optimization
    ModuleStats *mStatsBefore, *mStatsAfter;
    
    // Constructor, doesn't do anything
    ProjectZero() : ModulePass(ID) {}
    
    virtual bool runOnModule(Module &M) {
      mStatsBefore = getStats(M);
      bool modified = removeUnusedBlocks(M);
      mStatsAfter = getStats(M);
      return modified;
    }

    // Removes any unreachable basic blocks from the module.
    // Returns true if the module was modified.
    bool removeUnusedBlocks(Module &M) {
      bool modified = false;
      // Get a list of the functions in this module
      Module::FunctionListType &funcs = M.getFunctionList();
      // Iterate over the functions in this module
      for (Module::iterator F = funcs.begin(), E = funcs.end(); F != E; ++F) {
        // We are only interested in function definitions, not declarations
        if (!F->isDeclaration()) {
          modified |= removeUnusedBlocks(F);
        }
      }
      return modified;
    }

    // removes any unreachable basic blocks from the function;
    // returns true if the function was modified
    bool removeUnusedBlocks(Function *F) {
      bool modified;
      // Reverse the reachability graph
      map<BasicBlock*, set<BasicBlock*>*> *reaching = transpose(getReach(F), F);
      for (map<BasicBlock*, set<BasicBlock*>*>::iterator I = reaching->begin(); I != reaching->end(); ++I) {
        BasicBlock *B = I->first;
        // The set of blocks that can reach B
        set<BasicBlock*> *reachingB = I->second;
        // If the entry block cannot reach B, then we can remove the block
        if (reachingB->find(&(F->getEntryBlock())) == reachingB->end()) {
          B->eraseFromParent();
          modified = true;
        }
      }
      return modified;
    }

    ModuleStats *getStats(Module &M) {
      // Construct an empty module statistics struct
      ModuleStats *mStats = new ModuleStats();
      // Get a list of the functions in this module
      Module::FunctionListType &funcs = M.getFunctionList();
      // Iterate over the functions in this module
      for (Module::iterator F = funcs.begin(), E = funcs.end(); F != E; ++F) {
        // We are only interested in function definitions, not declarations
        if (!F->isDeclaration()) {
          // Compute the function stats and add them to the module stats
          mStats->add(getStats(F));
        }
      }
      return mStats;
    }

    FunctionStats *getStats(Function *F) {
      // Construct empty statistics object for this function
      FunctionStats *fStats = new FunctionStats();
      // Get dominator tree analysis for this function
      map<BasicBlock*, set<BasicBlock*>*> *dom = getDom(F);
      // Get loop analysis for this function
      map<BasicBlock*, set<BasicBlock*>*> *reach = getReach(F);
      // B: Iterate over the basic blocks in this function
      for (Function::iterator I = F->begin(), E = F->end(); I != E; ++I) {
        BasicBlock *B = I;
        // Construct empty statistics object for this block
        BasicBlockStats *bStats = new BasicBlockStats();
        // Increment the count of basic blocks in the function
        ++(fStats->basicBlocks);
        // Get the block's terminator (the last instruction in the block)
        TerminatorInst *T = B->getTerminator();
        // Record the number of successors of this block
        bStats->successors = T->getNumSuccessors();
        // Blocks reachable from B
        set<BasicBlock*>* bReach = (*reach)[B];
        // B2: Iterate over the blocks in the function again
        for (Function::iterator B2 = F->begin(), K = F->end(); B2 != K; ++B2) {
          // Blocks reachable from B2
          set<BasicBlock*>* b2Reach = (*reach)[B2];
          // If there is a path from B to B2, and a path from B2 to B, this is in a loop
          if (bReach->find(B2) != bReach->end() && b2Reach->find(B) != b2Reach->end()) bStats->isLoopBlock = 1;
          // Does this block dominate the block we're generating stats for (does B2 dominate B)?
          if ((*dom)[B]->find(B2) != (*dom)[B]->end()) {
            // Increment the number of dominator tree ancestors of the block
            ++(bStats->domTreeAncestors);
            // i: Iterate over the indices of B's successors
            for (int i = 0; i < T->getNumSuccessors(); i++) {
              // if B2 is in B's successor list and B2 dominates B, we found a single entry loop
              if (T->getSuccessor(i) == B2) ++(fStats->singleEntryLoops);
            }
          }
          // If the block we're generating stats for dominate this block (B dominates B2),
          // increment the number of dominator tree descendents of the block
          if ((*dom)[B2]->find(B) != (*dom)[B2]->end()) ++(bStats->domTreeDescendents);
        }
        fStats->add(bStats);
      }
      // This will become a list of all defined functions that are called by this function
      set<Function*> callees;
      // Iterate over the instructions in this function
      for (inst_iterator I = inst_begin(*F), E = inst_end(*F); I != E; ++I) {
        // If the instruction is a function call
        if (CallInst *caller = dyn_cast<CallInst>(&*I)) {
          // Get the callee
          if (Function *callee = caller->getCalledFunction()) {
            // If the callee is a function definition, not a declaration
            if (!callee->isDeclaration()) {
              // Add the callee to the set of callees
              callees.insert(callee);
            }
          }
        }
      }
      fStats->functionCalls += callees.size();
      return fStats;
    }
    
    virtual void print(raw_ostream &O, const Module *M) const {
      O << "\nBefore:\n" << *mStatsBefore << "\nAfter:\n" << *mStatsAfter << "\n";
    }
    
  };
  
}

char ProjectZero::ID = 0;
INITIALIZE_PASS(ProjectZero, "phase0-gth773s", "CS 6241, Phase 0, gth773s", false, false);

