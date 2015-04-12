#include "llvm/Pass.h"
#include "llvm/IR/Function.h"             
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include <vector>
#include <algorithm> 

using namespace llvm;
using namespace std;

namespace {
    StringRef getGlobalStringConstant(Value *val) {
        if (val->getValueID() != Value::ConstantExprVal) {
            outs() << val->getValueID() << "\t" << Value::ConstantExprVal;
            return StringRef();
        }
        ConstantExpr *ce = (ConstantExpr *)val;
        GlobalVariable *gv = (GlobalVariable *)ce->getOperand(0);
        return cast<ConstantDataArray>(gv->getInitializer())->getAsString();
    }

    class LoopMemoryAnalysis : public FunctionPass {
        public:
            static char ID;

            LoopMemoryAnalysis() : FunctionPass(ID) {}

            ScalarEvolution  *SE;

            // Interested data structures
            std::vector<Value*> structures;

            void getAnalysisUsage(AnalysisUsage &AU) const override {
                AU.addRequired<ScalarEvolution>();
                AU.addRequired<LoopInfo>();

                AU.setPreservesCFG();
                AU.addPreserved<ScalarEvolution>();
            }

            virtual bool runOnLoop(Loop *L) {

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

                            const Value *pointer_operand = SI->getPointerOperand();

                            // GetElementPtr Instruction
                            if (const GEPOperator *GEP = dyn_cast<GEPOperator>(pointer_operand)) {

                                const Value* pointer_operand = GEP->getPointerOperand();

                                //if(std::count(structures.begin(), structures.end(), pointer_operand) == 0)
                                //    continue;

                                outs() << "Pointer : ";
                                pointer_operand->print(outs());
                                outs() << "\n";


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

            virtual bool runOnFunction(Function &F) {
                if (F.size() == 0)
                    return false;

                outs() << F.getName() << ":\n\n";

                // ANNOTATION ANALYSIS

                // Annotations always in the entry block
                std::map<Value *, Value *> mapValueToArgument;
                BasicBlock *b = &F.getEntryBlock();

                //Build pointer to value mapping
                for(BasicBlock::iterator it = b->begin(); it!=b->end();++it) {
                    Instruction *inst = it;
                    if (inst->getOpcode()!=Instruction::BitCast) {
                        continue;
                    }

                    mapValueToArgument[inst] = inst->getOperand(0);
                }

                for (auto it = b->begin(); it != b->end(); it++) {
                    Instruction *I = it;
                    if (I->getOpcode() != Instruction::Call) {
                        continue;
                    }
                    Value *calledFunction = I->getOperand(I->getNumOperands()-1);
                    if(calledFunction->getName().str() != "llvm.var.annotation")
                        continue;
                    Value * annotatedValue = I->getOperand(0);
                    if (mapValueToArgument.count(annotatedValue)) {
                        annotatedValue = mapValueToArgument[annotatedValue];
                    }

                    Value *annotation = I->getOperand(1);
                    Value *filename = I->getOperand(2);
                    Value *linenumber = I->getOperand(3);

                    outs() << getGlobalStringConstant(filename) << ":" << *linenumber << " " <<
                        "\t" << *annotatedValue << "\t" << getGlobalStringConstant(annotation) << "\n";

                    structures.push_back(annotatedValue);
                }

                // LOOP ANALYSIS
                LoopInfo &LI = getAnalysis<LoopInfo>();
                for (LoopInfo::iterator loop_itr = LI.begin(), e = LI.end(); loop_itr != e; ++loop_itr) {
                    runOnLoop(*loop_itr);
                }

                return false;
            }

    };
}

char LoopMemoryAnalysis::ID = 0;

static RegisterPass<LoopMemoryAnalysis> X("loop-memory-analysis", "Analyze memory accesses within loops", false, false);
