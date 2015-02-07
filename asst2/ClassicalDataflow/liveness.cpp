// 15-745 S15 Assignment 2: liveness.cpp
// Group: jarulraj, nkshah
////////////////////// //////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

#include "dataflow.h"

using namespace llvm;

namespace {

  class Liveness : public FunctionPass, DataFlow {
  public:
    static char ID;

    Liveness() : FunctionPass(ID) { }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

    TransferOutput transferFn(BitVector input, DenseMap<Value*, int> domainToIndex, BasicBlock* block)
    {
      TransferOutput tempTransferOutput;

      return tempTransferOutput;
    }

  private:

    virtual bool runOnFunction(Function &F) {
      bool modified = false;

      // Print Information
      std::string function_name = F.getName();
      DBG(outs() << "FUNCTION :: " << function_name  << "\n");

      DataFlowResult df_result;
      //df_result = run(F, domain, Direction , MeetOp meet, BitVector boundaryCond, BitVector initCond);

      return modified;
    }

    virtual bool runOnModule(Module& M) {
      bool modified = false;

      // Run analysis on each function
      for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
        if(!modified)
          modified = runOnFunction(*MI);
      }

      return modified;
    }

  };

  char Liveness::ID = 0;
  RegisterPass<Liveness> X("liveness", "15745 Liveness");

}
