#define DEBUG_TYPE "projectloops"
#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/Dominators.h"
//#include "llvm/Function.h"
//#include "llvm/Instructions.h"
//#include "llvm/IntrinsicInst.h"
//#include "llvm/Constant.h"
//#include "llvm/Type.h"
//#include "llvm/Analysis/AliasAnalysis.h"
/*#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ValueHandle.h"*/
using namespace llvm;

namespace {

  struct MinMaxTotal {
    int min, max, total;
    MinMaxTotal() {
      min = max = total = 0;
    }
    void add(int a) {
      if (a < min) min = a;
      if (a > max) max = a;
      total += a;
    }
    void print(raw_ostream &O) {
      O << "min " << min << ", max " << max << ", total " << total;
    }
  };

  raw_ostream &operator<<(raw_ostream &o, MinMaxTotal &mmt) {
    mmt.print(o);
    return o;
  }

  struct ProjectZero : public ModulePass {

    static char ID; // Pass identification, replacement for typeid

    int NumberOfFunctions;
    int NumberOfFunctionCalls;
	MinMaxTotal* NumberOfBackEdges;
    MinMaxTotal* NumberOfBasicBlocks;

    ProjectZero() : ModulePass(ID) {
      NumberOfFunctions = NumberOfFunctionCalls = 0;
      NumberOfBasicBlocks = new MinMaxTotal();
	  NumberOfBackEdges = new MinMaxTotal();
    }

    virtual bool runOnModule(Module &M) {
      // Iterate over the functions in this module
      const Module::FunctionListType &funcs = M.getFunctionList();
      Module::const_iterator I  = funcs.begin();
      Module::const_iterator IE = funcs.end();
      for (; I != IE; ++I) {
        const Function &F = *I;
        runOnFunction(F);
      }
      return false;
    }

    void runOnFunction(const Function &F) {

      // Increment the number of functions
      ++NumberOfFunctions;

      int BasicBlocksInThisFunction = 0;

      // Iterate over the basic blocks in this function
      for (Function::const_iterator I = F.begin(), E = F.end(); I != E; ++I) {
        ++BasicBlocksInThisFunction;
      }

      NumberOfBasicBlocks->add(BasicBlocksInThisFunction);

      // Iterate over the instructions in this function
      for (const_inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        // If the instruction is a function call
        if (const CallInst *call = dyn_cast<CallInst>(&*I)) {
          // Increment the total number of function calls
          ++NumberOfFunctionCalls;
        }
      }
		
		if(BasicBlocksInThisFunction!=0){
			findBackEdges(F);
		}	
	}
	  
	  
	  ///////////////////////////////////	
	  //look for back edges in function
	  /////////////////////////////////
	  
	  void findBackEdges(const Function &F){
		  int BackEdgesInThisFunction = 0;
		  
		  const BasicBlock *BB = &F.getEntryBlock();
		  if (succ_begin(BB) == succ_end(BB))
			  return;
		  
		  SmallPtrSet<const BasicBlock*, 8> Visited;
		  SmallVector<std::pair<const BasicBlock*, succ_const_iterator>, 8> VisitStack;
		  SmallPtrSet<const BasicBlock*, 8> InStack;
		  
		  Visited.insert(BB);
		  VisitStack.push_back(std::make_pair(BB, succ_begin(BB)));
		  InStack.insert(BB);
		  
		  do{
			  std::pair<const BasicBlock*, succ_const_iterator> &Top = VisitStack.back();
			  const BasicBlock *ParentBB = Top.first;
			  succ_const_iterator &I = Top.second;
			  
			  bool FoundNew = false;
			  while(I != succ_end(ParentBB)){
				  BB = *I++;
				  
				  if(Visited.insert(BB)){
					  FoundNew = true;
					  break;
				  }
				  
				  //successor is in visitstack, its a back edge
				  if(InStack.count(BB))
					  BackEdgesInThisFunction++;
			  }
			  
			  if(FoundNew){
				  //Go down one level if there is an unvisited successor
				  InStack.insert(BB);
				  VisitStack.push_back(std::make_pair(BB,succ_begin(BB)));
			  }
			  
			  else{
				  //Go up one level
				  InStack.erase(VisitStack.pop_back_val().first);
			  }
			  
		  }while(!VisitStack.empty());
		  
		  NumberOfBackEdges->add(BackEdgesInThisFunction);
		  
	  }
		

    virtual void print(raw_ostream &O, const Module *M) const {
	  O << "Testing Project Loop Detection \n";
      O << "Functions: " << NumberOfFunctions << "\n";
      O << "Functions calls: " << NumberOfFunctionCalls << "\n";
      O << "Basic blocks: " << *NumberOfBasicBlocks << "\n";
	  O << "BackEdges: " << *NumberOfBackEdges << "\n";
    }

  };
}

char ProjectZero::ID = 0;
INITIALIZE_PASS(
  ProjectZero, "projectloops", "CS 6241 Project 0", false, false);

