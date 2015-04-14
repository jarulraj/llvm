#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DL_NAME "delinearize"
#define DEBUG_TYPE DL_NAME

namespace {

    class Delinearization : public FunctionPass {
        Delinearization(const Delinearization &); // do not implement
        protected:
        Function *F;
        LoopInfo *LI;
        ScalarEvolution *SE;

        public:
        static char ID; // Pass identification, replacement for typeid

        Delinearization() : FunctionPass(ID) {
            initializeDelinearizationPass(*PassRegistry::getPassRegistry());
        }
        bool runOnFunction(Function &F) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        void print(raw_ostream &O, const Module *M = nullptr) const override;
    };

} // end anonymous namespace

void Delinearization::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfo>();
    AU.addRequired<ScalarEvolution>();
}

bool Delinearization::runOnFunction(Function &F) {
    this->F = &F;
    SE = &getAnalysis<ScalarEvolution>();
    LI = &getAnalysis<LoopInfo>();

    print(outs(), nullptr);

    return false;
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

void Delinearization::print(raw_ostream &O, const Module *) const {
    O << "Delinearization on function " << F->getName() << ":\n";
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        Instruction *Inst = &(*I);

        // Only analyze loads and stores.
        if (!isa<StoreInst>(Inst) && !isa<LoadInst>(Inst) &&
                !isa<GetElementPtrInst>(Inst))
            continue;

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
            O << "Inst:" << *Inst << "\n";
            O << "In Loop with Header: " << L->getHeader()->getName() << "\n";
            O << "AddRec: " << *AR << "\n";

            SmallVector<const SCEV *, 3> Subscripts, Sizes;
            AR->delinearize(*SE, Subscripts, Sizes, SE->getElementSize(Inst));
            if (Subscripts.size() == 0 || Sizes.size() == 0 ||
                    Subscripts.size() != Sizes.size()) {
                O << "failed to delinearize\n";
                continue;
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
            O << "\n";
        }
    }
}

char Delinearization::ID = 0;
static RegisterPass<Delinearization> X("delinearize-analysis", "Delinearize memory accesses within loops", false, false);
