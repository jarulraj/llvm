#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <algorithm>

using namespace llvm;
using namespace std;

#define DL_NAME "loop-memory-analysis"
#define DEBUG_TYPE DL_NAME

namespace {


    class LoopMemoryAnalysis : public FunctionPass {
        LoopMemoryAnalysis(const LoopMemoryAnalysis &); // do not implement

        protected:
        LoopInfo *LI;
        ScalarEvolution *SE;

        // Interested data structures
        std::vector<Value*> structures;

        public:
        static char ID; // Pass identification, replacement for typeid

        LoopMemoryAnalysis() : FunctionPass(ID) {
        }

        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool runOnFunction(Function &F) override;
    };
}

void LoopMemoryAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();

    AU.addRequired<LoopInfo>();
    AU.addRequired<ScalarEvolution>();
}

static StringRef getGlobalStringConstant(raw_ostream& O, Value *val) {
    if (val->getValueID() != Value::ConstantExprVal) {
        O << val->getValueID() << "\t" << Value::ConstantExprVal;
        return StringRef();
    }
    ConstantExpr *ce = (ConstantExpr *)val;
    GlobalVariable *gv = (GlobalVariable *)ce->getOperand(0);
    return cast<ConstantDataArray>(gv->getInitializer())->getAsString();
}

static Value *getPointerOperand(Instruction &Inst) {
    if (LoadInst *Load = dyn_cast<LoadInst>(&Inst))
        return Load->getPointerOperand();
    else if (StoreInst *Store = dyn_cast<StoreInst>(&Inst))
        return Store->getPointerOperand();
    else if (GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(&Inst))

        return Gep->getPointerOperand();
    return nullptr;
}           

void parseExpression(const SCEVAddRecExpr* AR, std::vector<APInt> constants) {

    raw_ostream &O = outs();
	O << "Call parseExpression\n";
	/*
	 * The outermost braces correspond to the innermost loop.
	 */
	if(AR->getNumOperands() != 2) {
		O << "Too many operands in SCEVAddExpr\n";
		exit(-1);
	}

	const SCEV *operand_0, *operand_1;
	
	operand_0 = AR->getOperand(0);
	operand_1 = AR->getOperand(1);
	O << "Operands: " << *operand_0 << ", " << *operand_1 << "\n";

	if(operand_1->getSCEVType() == llvm::scConstant) {
		ConstantInt *val_1 = ((SCEVConstant*) operand_1)->getValue();
		const APInt& ap_val_1 = val_1->getValue();

		O << "Detected strided access with stride = " << ap_val_1 << "\n";
		constants.push_back(ap_val_1);

		/**< Push twice to avoid segfault */
		constants.push_back(ap_val_1);
	}
}

bool LoopMemoryAnalysis::runOnFunction(Function &F) {
    SE = &getAnalysis<ScalarEvolution>();
    LI = &getAnalysis<LoopInfo>();
    raw_ostream &O = outs();

    O << "Analysing function :: " << F.getName() << ":\n";

    // ANNOTATION 

    // Annotations always in the entry block
    std::map<Value *, Value *> mapValueToArgument;
    std::map<Value *, Value *> mapAccessToVar;
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

        O << getGlobalStringConstant(O, filename) << ":" << *linenumber << " " <<
            "\t" << *annotatedValue << "\t" <<
			getGlobalStringConstant(O, annotation) << "\n";

        structures.push_back(annotatedValue);
    }

    // DELINEARIZATION ANALYSIS

    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        Instruction *Inst = &(*I);

        if (GEPOperator *GEP = dyn_cast<GEPOperator>(Inst)) {
            Value *pointer_operand = GEP->getPointerOperand();
            if (std::count(structures.begin(), structures.end(), pointer_operand)) {
                mapAccessToVar[GEP] = pointer_operand;
                outs() << *GEP << " points to " << *pointer_operand << "\n";
            } else if (mapAccessToVar.count(pointer_operand)) {
                mapAccessToVar[GEP] = mapAccessToVar[pointer_operand];
                //outs() << *GEP << " points to " << *ptr_map[pointer_operand] << "\n";
            }
            continue;
        }

        // Only analyze loads and stores.
        if (!isa<StoreInst>(Inst) && !isa<LoadInst>(Inst) &&
                !isa<GetElementPtrInst>(Inst))
            continue;

		Value *pointer_operand;
		if (isa<StoreInst>(Inst)) {
			StoreInst *si = dyn_cast<StoreInst>(Inst);
			pointer_operand = si->getPointerOperand();
		} else {
			LoadInst *li = dyn_cast<LoadInst>(Inst);
			pointer_operand = li->getPointerOperand();
		}

		if (mapAccessToVar.count(pointer_operand) == 0 &&
			std::count(structures.begin(), structures.end(), pointer_operand) == 0) {
			continue;
		}

        if (mapAccessToVar.count(Inst) != 0) {
            continue;
        }

        const BasicBlock *BB = Inst->getParent();
        // Delinearize the memory access as analyzed in all the surrounding loops.
        // Do not analyze memory accesses outside loops.
        for (Loop *L = LI->getLoopFor(BB); L != nullptr; L = L->getParentLoop()) {
            const SCEV *AccessFn = SE->getSCEVAtScope(getPointerOperand(*Inst), L);

            const SCEVUnknown *BasePointer =
                dyn_cast<SCEVUnknown>(SE->getPointerBase(AccessFn));
            // Do not delinearize if we cannot find the base pointer.
            if (!BasePointer)
                break;
            AccessFn = SE->getMinusSCEV(AccessFn, BasePointer);
            const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(AccessFn);

            // Do not try to delinearize memory accesses that are not AddRecs.
            if (!AR)
                break;

            O << "\n";
            O << "Inst: " << *Inst << "\n";
            O << "Operand: " << *getPointerOperand(*Inst) << "\n";
            O << "In Loop with Header: " << L->getHeader()->getName() << "\n";
            O << "AddRec: " << *AR << "\n";

            // Cost model
            std::vector<APInt> constants;
            parseExpression(AR, constants);

            // Col vs Row
            APInt last = constants.back();
            constants.pop_back();
            APInt penultimate = constants.back();
            
            if(last.sle(penultimate)) {
                O << "//====------------------------------------------------===//\n";
                O << "// Regular Access :: Pattern ID 0 \n";
                O << "//====------------------------------------------------===//\n";
            }
            else {
                O << "//====------------------------------------------------===//\n";
                O << "// Columnar Access :: Pattern ID 1 \n";
                O << "//====------------------------------------------------===//\n";
            }

            /*SmallVector<const SCEV *, 3> Subscripts, Sizes;
            AR->delinearize(*SE, Subscripts, Sizes, SE->getElementSize(Inst));
            if (Subscripts.size() == 0 || Sizes.size() == 0 ||
                    Subscripts.size() != Sizes.size()) {
                O << "failed to delinearize\n\n";
                // Stop after innermost loop
                break;
            }

            O << "Base offset: " << *BasePointer << "\n";
            O << "ArrayDecl[UnknownSize]";
            int Size = Subscripts.size();
            for (int i = 0; i < Size - 1; i++)
                O << "[" << *Sizes[i] << "]";
            O << " with elements of " << *Sizes[Size - 1] << " bytes.\n";

            O << "ArrayRef";
            for (int i = 0; i < Size; i++)
                O << "[" << *Subscripts[i] << "]";
            O << "\n\n";*/

            // Stop after innermost loop
            break;
        }
    }

    // only analysis for now
    return false;
}


char LoopMemoryAnalysis::ID = 0;

static RegisterPass<LoopMemoryAnalysis> X("loop-memory-analysis",
	"Analyze memory accesses within loops", false, false);
