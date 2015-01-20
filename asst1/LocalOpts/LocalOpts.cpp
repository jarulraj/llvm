// 15-745 S15 Assignment 1: LocalOpts.cpp
// Group: jarulraj, nkshah

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

// Get constant from value
#define get_const(t,val) ConstantInt::get((IntegerType*)t,val)

// DEBUG mode: Prints all the optimizations performed to stdout, and the original and final code so we can compare!
#define DEBUG 1

#ifdef DEBUG
	#define DBG(a) a
#else
	#define DBG(a) 
#endif

namespace
{
	// Struct storing statistics about the number of different optimizations performed
	struct LocalOptsInfoStruct {
		LocalOptsInfoStruct() : numAlgebraicOpts(0), numConstantFolds(0), numStrengthReds(0) {}
		int numAlgebraicOpts;
		int numConstantFolds;
		int numStrengthReds;
	} LocalOptsInfo;
	
	class LocalOpts : public ModulePass
	{

		// Algebraic Optimizations
		void algebraic_optimizations(BasicBlock& B)
		{
		}

		// Constant Folding
		void constant_folding(BasicBlock& B)
		{
			// Iterate over all instructions
			for(BasicBlock::iterator BI = B.begin(), BE = B.end(); BI != BE; ) {
				Instruction& inst(*BI);
				bool inst_removed = false;	// Is this instruction removed due to constant folding?
				
				// In binary operations
				if(BinaryOperator::classof(&inst)) {
					BinaryOperator *bop((BinaryOperator*)&inst);
					Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
					Value *val2(bop->getOperand(1));
					if(!ConstantInt::classof(val1) || !ConstantInt::classof(val2)) continue; // The values are not integers, then there's nothing to do!
					ConstantInt *ci1 = dyn_cast<ConstantInt>(val1);	// Get constants from values
					ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);

					switch (bop->getOpcode()) {	// Fold depending on what operator is used
					
					case Instruction::Add: {	// Addition
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) + (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " + " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::Sub: {	// Subtraction
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) - (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " - " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;
						
					case Instruction::Mul: {	// Multiplication
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) * (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " * " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::SDiv: {	// Division
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) / (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " / " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::Shl: {	// Left Shift
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) << (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " << " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::LShr: {	// (Logical) Right Shift
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) >> (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " >> " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;
					}
				}
				
				// Increment the iterator only if the current instruction was not removed. 
				// This is chosen instead of doing --BI when we remove the instruction because --BI crashes when BI is at B.begin()
				if (!inst_removed) ++BI;	
			}
		}

		// Strength Reduction
		void strength_reduction(BasicBlock& B)
		{
		}
		
		// Printing summary statistics
		void printLocalOptsSummary() 
		{
			outs() << "Transformations applied:\n";
			outs() << "Algebraic identities: " << LocalOptsInfo.numAlgebraicOpts << "\n";
			outs() << "Constant folding: " << LocalOptsInfo.numConstantFolds << "\n";
			outs() << "Strength reduction: " << LocalOptsInfo.numStrengthReds << "\n";
		}

	public:
		static char ID;

		LocalOpts(): ModulePass(ID)
		{
		}

		~LocalOpts()
		{
		}

		// We don't modify the program, so we preserve all analyses
		virtual void getAnalysisUsage(AnalysisUsage &AU) const
		{
			AU.setPreservesCFG();
		}

		virtual bool runOnModule(Module& M)
		{
			for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI)
			{
				Function& F(*MI);

				for(Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
					BasicBlock& B(*FI);
					
					// In the debug mode, print the original code to stdout
					DBG(outs() << "ORIGINAL CODE:\n\n");
					DBG(B.dump());
					
					// In the debug mode, print every optimization performed to stdout
					DBG(outs() << "\nOPTIMIZATIONS PERFORMED:\n\n");
					algebraic_optimizations(B);
					constant_folding(B);
					strength_reduction(B);
					
					// In the debug mode, print the final code to stdout
					DBG(outs() << "\nFINAL CODE:\n\n");
					DBG(B.dump());
				}
			}
			
			// Print the summary statistics to stdout
			printLocalOptsSummary();
			return true;
		}
	};

	// Register the pass
	char LocalOpts::ID = 0;
	RegisterPass<LocalOpts> X("local-opts", "15745: Local Optimizations");
}
