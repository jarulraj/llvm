#include "llvm/Pass.h"
#include "llvm/IR/Function.h"             
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm;
using namespace std;

namespace {
    class LoopMemoryAnalysis : public LoopPass {
        public:
            static char ID;

            LoopMemoryAnalysis() : LoopPass(ID) {}

            ScalarEvolution  *SE;

            void getAnalysisUsage(AnalysisUsage &AU) const override {
                  AU.addRequired<ScalarEvolution>();

                  AU.setPreservesCFG();
                  AU.addPreserved<ScalarEvolution>();
            }

            virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {

                bool changed = false;

                outs() << "\n-------------------------------------------------------------\n";
                outs() << "Loop \n";
                L->print(outs());

                SE = &getAnalysis<ScalarEvolution>();

                for (Loop::block_iterator I = L->block_begin(), E = L->block_end();  I != E; ++I) {
                    BasicBlock *BB = *I;

                    for(BasicBlock::iterator I = BB->begin(); I != BB->end(); I++) {

                        if(StoreInst *SI = dyn_cast<StoreInst>(I)) {
                            outs() << "Store : ";
                            SI->print(outs());
                            outs() << "\n";

                            Value *operand = SI->getPointerOperand();

                            // GetElementPtr Instruction
                            if (const GEPOperator *GEP = dyn_cast<GEPOperator>(operand)) {
                                for (GEPOperator::const_op_iterator idx = GEP->idx_begin(), end = GEP->idx_end(); idx != end; idx++) {
                                    const SCEV *index = SE->getSCEV(*idx);

                                    outs() << "Index : ";
                                    index->print(outs());
                                    outs() << "\n";
                            
                                    if (const SCEVAddRecExpr *addRec = dyn_cast<SCEVAddRecExpr>(index)) {
                                    }
                                }
                            }

                        }

                        if(LoadInst *LI = dyn_cast<LoadInst>(I)) {
                            outs() << "Load : ";
                            LI->print(outs());
                            outs() << "\n";
                        }

                    }

                }

                return changed;
            }
    };
}

char LoopMemoryAnalysis::ID = 0;

static RegisterPass<LoopMemoryAnalysis> X("loop-memory-analysis", "Analyze memory accesses within loops", false, false);
