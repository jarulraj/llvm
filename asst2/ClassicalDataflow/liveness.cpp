// 15-745 S15 Assignment 2: liveness.cpp
// Group: jarulraj, nkshah
////////////////////// //////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

#include "dataflow.h"

using namespace llvm;

namespace {

  class Liveness : public FunctionPass {
  public:
    static char ID;

    Liveness() : FunctionPass(ID) {

      // Setup the pass
      Direction direction = Direction::BACKWARD;
      MeetOp meet_op = MeetOp::UNION;

      pass = LivenessAnalysis(direction, meet_op);
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }

  private:

    // Liveness Analysis class
    class LivenessAnalysis : public DataFlow {
    public:

      LivenessAnalysis()
        :        DataFlow(Direction::INVALID_DIRECTION,
                          MeetOp::INVALID_MEETOP)
      {
      }

      LivenessAnalysis(Direction direction, MeetOp meet_op)
        : DataFlow(direction, meet_op)
      {

      }

    protected:
      TransferOutput transferFn(BitVector input, std::map<void*, int> domainToIndex,
                                BasicBlock* block)
      {
        TransferOutput transferOutput;

        return transferOutput;
      }

    };

    // The pass
    LivenessAnalysis pass;

    virtual bool runOnFunction(Function &F) {
      bool modified = false;

      // Print Information
      std::string function_name = F.getName();
      DBG(outs() << "FUNCTION :: " << function_name  << "\n");
      DataFlowResult output;

      // Setup the pass
      std::vector<void*> domain;

      // Compute domain for function
      User::op_iterator OI, OE;

      for(Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
        BasicBlock& B(*FI);

        for(BasicBlock::iterator BI = B.begin(), BE = B.end(); BI != BE; ++BI) {
          Instruction& insn(*BI);

          // Look for instruction-defined values and function args
          for (OI = insn.op_begin(), OE = insn.op_end(); OI != OE; ++OI)
          {
            Value *val = *OI;
            if (isa<Instruction>(val) || isa<Argument>(val)) {
              // Val is used by insn
              domain.push_back(val);
            }
          }

        }

      }

      DBG(outs() << "------------------------------------------\n\n");
      DBG(outs() << "DOMAIN :: " << domain.size() << "\n");
      for(void* element : domain)
      {
        DBG(outs() << "Element : " << *((Value*) element) << "\n");
      }
      DBG(outs() << "------------------------------------------\n\n");

      // For LVA, both are empty sets
      BitVector boundaryCond(domain.size(), false);
      BitVector initCond(domain.size(), false);

      // Apply pass
      output = pass.run(F, domain, boundaryCond, initCond);

      printResult(output);

      return modified;
    }

    void printResult(DataFlowResult output)
    {
      for(auto entry : output.result)
      {
        DBG(outs() << "BB  " << entry.first->getName() << "\n");

        printBitVector(entry.second.in);
        printBitVector(entry.second.out);
      }
    }

    void printBitVector(BitVector b)
    {
      unsigned int i;
      unsigned int b_size = b.size();

      if(b_size == 0)
        DBG(outs() << "-");
      else
      {
        for(i = 0; i < b.size() ; i++)
        {
          if(b[i] == true)
            DBG(outs() << "1");
          else
            DBG(outs() << "0");
        }
      }
      DBG(outs() << "\n");
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
