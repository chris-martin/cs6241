#ifndef LLVM_PHASEONE
#define LLVM_PHASEONE

#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Transforms/Scalar.h"

#include <limits>
#include <list>

#include "Integer.h"
#include "LBound.h"
#include "Diff.h"
#include "Inequality.h"
#include "Pi.h"
#include "Comparison.h"
#include "Memo.h"
#include "Prover.h"
#include "AbcInserter.h"
#include "AbcRemover.h"

#endif
