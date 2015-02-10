// 15-745 S15 Assignment 2: liveness.cpp
// Group: jarulraj, nkshah
////////////////////// //////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstIterator.h"

#include "dataflow.h"
#include "available-support.h"
#include <sstream>

using namespace llvm;

namespace {

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

	void printResult(DataFlowResult output)
	{
		for(auto entry : output.result)
		{
			DBG(outs() << "BB " << entry.first->getName() << "\n");

			printBitVector(entry.second.in);
			printBitVector(entry.second.out);
		}
	}

	std::string printValue(Value* v)
	{
		std::string res; llvm::raw_string_ostream raw_st(res);
		v->print(raw_st);
		return res;
	}

	std::string printSet(std::vector<void*> domain, BitVector liveSet) {
		std::stringstream ss;
		ss << "{";
		int ind = 0;
		for (int i = 0; i < domain.size(); i++) {
			// If variable i is live
			if (liveSet[i]) {
				if (ind > 0) ss << ",";
				ss << " " << getShortValueName((Value*)domain[i]);
				ind++;
			}
		}
		ss << " }";
		return ss.str();
	}

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

		LivenessAnalysis() : DataFlow(Direction::INVALID_DIRECTION, MeetOp::INVALID_MEETOP) {	}
		LivenessAnalysis(Direction direction, MeetOp meet_op) : DataFlow(direction, meet_op) { }
		
    protected:
      TransferOutput transferFn(BitVector input, std::map<void*, int> domainToIndex, BasicBlock* block)
      {
      
        TransferOutput transferOutput;

		    // Calculating the set of locally exposed uses and variables defined
		    int domainSize = domainToIndex.size();
		    BitVector defSet(domainSize);
		    BitVector useSet(domainSize);
		    for (BasicBlock::iterator insn = block->begin(); insn != block->end(); ++insn) {
		      // Locally exposed uses
		      
		      // Phi nodes: add operands to the list we store in transferOutput
		      if (PHINode* phi_insn = dyn_cast<PHINode>(&*insn)) {
		        for (int ind = 0; ind < phi_insn->getNumIncomingValues(); ind++) {
		        
		          Value* val = phi_insn->getIncomingValue(ind);
		          if (isa<Instruction>(val) || isa<Argument>(val)) {
		          
		            BasicBlock* valBlock = phi_insn->getIncomingBlock(ind);
		            if (transferOutput.neighborVals.find(valBlock) == transferOutput.neighborVals.end())	// neighborVals has no mapping for this block, then create one
		              transferOutput.neighborVals[valBlock] = BitVector(domainSize);
		            int valInd = domainToIndex[(void*)val];
		            transferOutput.neighborVals[valBlock].set(valInd);	// Set the bit corresponding to "val"
		            
		          }
		        }
		      }
		      
		      //Non-phi nodes: Simply add operands to the use set
		      else {
		        for (User::op_iterator opnd = insn->op_begin(), opE = insn->op_end(); opnd != opE; ++opnd) {
		          Value* val = *opnd;
		          if (isa<Instruction>(val) || isa<Argument>(val)) {
		            int valInd = domainToIndex[(void*)val];
		            if (!defSet[valInd])	// Add to useSet only if not already defined in the block somewhere earlier
		              useSet.set(valInd);
		          }
		        }
		      }

		      // Definitions
		      std::map<void*, int>::iterator iter = domainToIndex.find((void*)insn);
		      if (iter != domainToIndex.end())
		        defSet.set((*iter).second);
		    }

		    // Transfer function = useSet U (input - defSet)
		    transferOutput.element = defSet;
		    transferOutput.element.flip();	// Complement of defSet
		    transferOutput.element &= input;	// input - defSet = input INTERSECTION Complement of defSet
		    transferOutput.element |= useSet;

        return transferOutput;
      }

    };

    // The pass
    LivenessAnalysis pass;

    virtual bool runOnFunction(Function &F) {
      // Print Information
      std::string function_name = F.getName();
      DBG(outs() << "FUNCTION :: " << function_name  << "\n");
      DataFlowResult output;

      // Setup the pass
      std::vector<void*> domain;

      // Compute domain for function

      for(inst_iterator II = inst_begin(F), IE = inst_end(F); II!=IE; ++II) {
      	Instruction& insn(*II);
        // Look for insn-defined values and function args
        for (User::op_iterator OI = insn.op_begin(), OE = insn.op_end(); OI != OE; ++OI)
        {
          Value *val = *OI;
          if (isa<Instruction>(val) || isa<Argument>(val)) {
            // Val is used by insn
            if(std::find(domain.begin(),domain.end(),val) == domain.end())
	            domain.push_back((void*)val);
          }
        }
      }

      DBG(outs() << "------------------------------------------\n\n");
      DBG(outs() << "DOMAIN :: " << domain.size() << "\n");
      for(void* element : domain)
      {
        DBG(outs() << "Element : " << *((Value*) element) << "\n"); // Could also use getShortValueName((Value*) element)
      }
      DBG(outs() << "------------------------------------------\n\n");

      // For LVA, both are empty sets
      BitVector boundaryCond(domain.size(), false);
      BitVector initCond(domain.size(), false);

      // Apply pass
      output = pass.run(F, domain, boundaryCond, initCond);
      //printResult(output);

		  // We use the results to compute the final liveness (we handle phi nodes here)
		  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
		  	BasicBlock* block = BI;

		    // liveness at OUT
		    BitVector liveValues = output.result[block].out;
				
				// Generate Print Information in Reverse Order
		    std::vector<std::string> revOut;

        // Print live variables at the end of the block
        revOut.push_back("------------------------------------------------------");
        revOut.push_back("\nOUT Live Set: " + printSet(domain, liveValues) + "\n");

		    // Iterate backward through the block, update liveness
		    for (BasicBlock::reverse_iterator insn = block->rbegin(), IE = block->rend(); insn != IE; ++insn) {

		      // Add the instruction itself
		      revOut.push_back(printValue(&*insn));

		      // Phi inst: Kill LHS, but don't output liveness here
		      if (PHINode* phiInst = dyn_cast<PHINode>(&*insn)) {
		        std::map<void*, int>::const_iterator it = output.domainToIndex.find((void*)phiInst);
		        if (it != output.domainToIndex.end())
		          liveValues.reset(it->second);
		      }
		      else {
		        // Make values live when used as operands
		        for (Instruction::op_iterator opnd = insn->op_begin(), opE = insn->op_end(); opnd != opE; ++opnd) {
		          Value* val = *opnd;
		          if (isa<Instruction>(val) || isa<Argument>(val)) {
		            int ind = output.domainToIndex[(void*)val];
		            liveValues.set(ind);
		          }
		        }

		        // When a value is defined, remove it from live set before that instruction
		        std::map<void*, int>::iterator it = output.domainToIndex.find((void*)(&*insn));
		        if (it != output.domainToIndex.end())
		          liveValues.reset(it->second);

		        // Print live variables
		        revOut.push_back("\nLive Set: " + printSet(domain, liveValues) + "\n");
		      }
		    }

        revOut.push_back("------------------------------------------------------\n");
		    
		    // Since we added strings in the reverse order, print them in reverse
		    for (std::vector<std::string>::reverse_iterator it = revOut.rbegin(); it != revOut.rend(); ++it)
		      outs() << *it << "\n";
		  }
      return false;
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
