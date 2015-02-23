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
			AU.setPreservesAll();
		}
    
    virtual bool runOnLoop(Loop *L, LPPassManager &no_use) {
			printIdom(computeIdom(computeDom(L)),L);
			return false;
    }
};

//
// Register Pass
//
char LICM::ID = 0;
RegisterPass<LICM> Y("loop-invariant-code-motion", "15745 LICM");

}
