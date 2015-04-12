////////////////////////////////////////////////////////////////////////////////
// 15-745 Project: MemoryAccess.cpp
// Group: jarulraj, akalia, junwoop
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"



#include <ostream>
#include <fstream>
#include <iostream>

using namespace llvm;

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

    class MemoryAccess : public ModulePass {

        // Output the function information to standard out.
        void printMemoryAccess(Module& M) {
            outs() << "Module " << M.getModuleIdentifier().c_str() << "\n";

            // Print info about each function
            for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
                runOnFunction(*MI);
            }

        }

        public:

        static char ID;

        MemoryAccess() : ModulePass(ID) { }

        ~MemoryAccess() { }

        // We don't modify the program, so we preserve all analyses
        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
        }

        virtual bool runOnFunction(Function &F) {
            if (F.size() == 0)
                return false;
            outs() << F.getName() << ":\n\n";
            std::map<Value *, Value *> mapValueToArgument;
            // Annotations always in the entry block
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

                // `ConstantExpr` operands: http://llvm.org/docs/LangRef.html#constantexprs
                //         Value *gv = ce->getOperand(0);
                //
                //                 if(gv->getValueID() != Value::GlobalVariableVal)
                //                             continue;
                //

                Value * filename = I->getOperand(2);
                Value * linenumber = I->getOperand(3);
                outs() << getGlobalStringConstant(filename) << ":" << *linenumber << " " <<
                    "\t" << *annotatedValue << "\t" << getGlobalStringConstant(annotation) << "\n";

            }

            return false;
        }

        virtual bool runOnModule(Module& M) {
            printMemoryAccess(M);
            return false;
        }

    };

    // LLVM uses the address of this static member to identify the pass, so the
    // initialization value is unimportant.
    char MemoryAccess::ID = 0;
    RegisterPass<MemoryAccess> X("memory-access", "15745: Memory Access Optimization");
}
