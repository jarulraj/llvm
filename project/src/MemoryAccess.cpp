////////////////////////////////////////////////////////////////////////////////
// 15-745 Project: MemoryAccess.cpp
// Group: jarulraj, akalia, junwoop
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

#include <ostream>
#include <fstream>
#include <iostream>

using namespace llvm;

namespace {

class MemoryAccess : public ModulePass {

	// Output the function information to standard out.
	void printMemoryAccess(Module& M) {
		outs() << "Module " << M.getModuleIdentifier().c_str() << "\n";
		outs() << "Name,\tArgs,\tCalls,\tBlocks,\tInsns\n";

		// Print info about each function
		for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
			runOnFunction(*MI);
		}

		// Annotations
		for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E; ++I) {
			if (I->getName() == "llvm.global.annotations") {
				Value *V = cast<Value>(I->getOperand(0));
				outs()<<"\n Annotation :: "<<*(V)<<"\n";
				outs()<<"\n "<<*(V->getType())<<"\n";
			}
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
		bool is_var_arg = false;
		size_t arg_count = 0;
		size_t callsite_count = 0;
		size_t block_count = 0;
		size_t instruction_count = 0;

		// Get all the required information
		std::string function_name = F.getName(); // Get name
		is_var_arg = F.isVarArg(); // Check if # arguments is variable
		if (!is_var_arg) {
			arg_count = F.arg_size(); // # fixed args
		}

		callsite_count = F.getNumUses(); // # direct call sites
		block_count = F.size(); // # basic blocks

		// # instructions
		for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
			instruction_count += FI->size();
		}

		// Print Information
		outs() << function_name  << ",\t";
		if (is_var_arg) {
			outs() << "*,\t";
		} else {
			outs() << arg_count << ",\t";
		}
		outs() << callsite_count << ",\t" << block_count << ",\t"
				<< instruction_count << "\n";

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
