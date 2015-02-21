/////////////////////////////////////////
// 15-745 S15 Assignment 3:
// Group: nkshah, jarulraj
/////////////////////////////////////////

#ifndef __DOMINANCE_H
#define __DOMINANCE_H

#include <set>
#include <queue>
#include <vector>

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"

#include "dataflow.h"

using namespace std;

namespace llvm {

// Dominance Analysis
class DominanceAnalysis : public DataFlow {
	public:
      DominanceAnalysis() : DataFlow(Direction::FORWARD, MeetOp::INTERSECTION) {	}

  protected:

		// Transfer function is simple: GEN = {Self}, KILL = emptyset
    TransferOutput transferFn(BitVector input, std::vector<void*> domain, std::map<void*, int> domainToIndex, BasicBlock* block) {
      TransferOutput result;
      result.element = input;	// Get the IN set
      result.element.set(domainToIndex[block]);	// Add self
      return result;
    }
};

// Compute Dominance Relations
DataFlowResult computeDom(Loop* L);

// Function for checking if block X dominates block Y
bool dominates(BasicBlock* X, BasicBlock* Y, DataFlowResult dom);

std::map<BasicBlock*, BasicBlock*> computeIdom(DataFlowResult dominanceResult);

void printIdom(std::map<BasicBlock*, BasicBlock*> idom, Loop* L);

}

#endif
