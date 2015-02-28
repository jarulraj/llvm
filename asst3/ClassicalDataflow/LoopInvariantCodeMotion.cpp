/////////////////////////////////////////
// 15-745 S15 Assignment 3:
// Group: nkshah, jarulraj
/////////////////////////////////////////

#include <set>
#include <queue>
#include <vector>

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

#include "dataflow.h"
#include "dominance.h"

using namespace std;

namespace llvm {

class LICM : public LoopPass {
public:
    static char ID;

    LICM() : LoopPass(ID) {}

	virtual void getAnalysisUsage(AnalysisUsage& AU) const {
		AU.addRequired<LoopInfo>();
	}
		
    virtual bool runOnLoop(Loop *L, LPPassManager &no_use) {
		// 
		bool modified = false;

		// From the LLVM documentation, we know that LoopPass calls runOnLoop on the loops in the loop nest order, so the outermost loop is processed last. 
		// So we don't need to take care of LICM "bubbling" all the way through.
		
		// Ignore loops without a pre-header
		if (L->getLoopPreheader() == NULL)
		  return false;

		// Else, get the loop info
		LoopInfo& LI = getAnalysis<LoopInfo>();
		
		printIdom(computeIdom(computeDom(L)),L);

		set<Value*> loopInvariantStatements = computeLoopInvariantStatements(L, reachingDefs);

		set<Value*> codeMotionCandidateStatements = computeCodeMotionCandidateStatements(L, dominanceResults, loopInvariantStatements);

		bool loopModified = applyMotionToCandidates(L, codeMotionCandidateStatements);

		modified |= loopModified;

		LQ.pop_back();
		}

		return modified;
    }
};

//
// Register Pass
//
char LICM::ID = 0;
RegisterPass<LICM> Y("loop-invariant-code-motion", "15745 LICM");

}
