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
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LegacyPassManagers.h"

#include "dataflow.h"
#include "dominance.h"

using namespace std;

namespace llvm {

	class LICM : public LoopPass {
	public:
		static char ID;

		LICM() : LoopPass(ID) {}

		bool isInvariant(Loop* L, std::vector<Instruction*> invStmts, Instruction* inst) {

			// Conditions given in the assignment
			if (!isSafeToSpeculativelyExecute(inst) ||
			    inst->mayReadFromMemory() ||
			    isa<LandingPadInst>(inst)) {
				return false;
			}

			// All operands must be constants or loop invStmts themselves
			for (User::op_iterator OI = inst->op_begin(), OE = inst->op_end(); OI != OE; ++OI) {
				Value *op = *OI;
				if (Instruction* op_inst = dyn_cast<Instruction>(op)) {
					if (L->contains(op_inst) && std::find(invStmts.begin(), invStmts.end(), op_inst) == invStmts.end()) {	// op_inst in loop, and it is not loop invariant
						return false;
					}
				}
			}

			// If not returned false till now, then it is loop invariant
			return true;
		}

		virtual void getAnalysisUsage(AnalysisUsage& AU) const {
			AU.addRequired<LoopInfo>();
		}

		virtual bool runOnLoop(Loop *L, LPPassManager &no_use) {
			bool modified = false;

			// From the LLVM documentation, we know that LoopPass calls runOnLoop on the loops in the loop nest order, so the outermost loop is processed last.
			// So we don't need to take care of LICM "bubbling" all the way through.

			// Ignore loops without a pre-header
			BasicBlock* preheader = L->getLoopPreheader();
			if (!preheader) {
				return false;
			}

			// Else, get the loop info
			LoopInfo& LI = getAnalysis<LoopInfo>();

			DataFlowResult dominanceResult = computeDom(L);
			std::map<BasicBlock*,BasicBlock*> idom = computeIdom(dominanceResult);
			printIdom(idom,L);
			DomTree* tree = getDominanceTree(idom,L);

			std::vector<DomTreeNode*> worklist;
			worklist.push_back(tree->root);

			std::vector<Instruction*> invStmts;	// Loop-invariant statements

			// Traverse in DFS order, so don't need to do multiple iterations. Use worklist as a stack
			while (!worklist.empty()) {
				DomTreeNode* n = worklist.back();
				BasicBlock* b = n->block;
				worklist.pop_back();

				// Skip this block if it is part of a subloop (thus, already processed)
				if (LI.getLoopFor(b) != L) {
					return false;
				}

				// Iterate through all the intructions.
				for (BasicBlock::iterator II = b->begin(), IE = b->end(); II != IE; ++II) {
					Instruction* inst = &(*II);
					bool inv = isInvariant(L, invStmts, inst);
					if (inv) {
						invStmts.push_back(inst);
					}
				}

				for (int i = 0; i < n->children.size(); ++i) {
					worklist.push_back(n->children[i]);
				}
			}

			// Conditions for hoisting out of the loop
			// In SSA, everything is assigned only once, and must be done before all its uses. So only need to check if all loop exits are dominated.

			/* No more checking of dominating all loop exits
			// Find all loop exits
			std::vector<BasicBlock*> exitblocks;
			std::vector<BasicBlock*> blocks = L->getBlocks();
			for(int i=0; i<blocks.size(); ++i) {
			BasicBlock* BB = blocks[i];
			for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
			if (!L->contains(*SI)) {
			exitblocks.push_back(BB);
			break;
			}
			}
			}
			*/

			// Check if instruction can be moved, and do code motion in the order in which invStmts were added (while maintaining dependencies)
			for (int j = 0; j < invStmts.size(); ++j) {
				Instruction* inst = invStmts[j];
				BasicBlock* b = inst->getParent();

				bool all_dominate = true;
				/* No more checking of dominating all loop exits
				// Check if it dominates all loop exits
				for(int i=0; i<exitblocks.size(); ++i) {
				if (!dominates(b,exitblocks[i],dominanceResult)) {
				all_dominate = false;
				break;
				}
				}
				*/

				if (all_dominate) {
					Instruction* end = &(preheader->back());
					inst->moveBefore(end);
					if (!modified) {
						modified = true;
					}
				}
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
