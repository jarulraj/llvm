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
	
	// If n is a power of 2, then return that power, else return -1
	int64_t find_log (int64_t n) 
	{
		if (n <= 0) return -1;
		int64_t res = 0;
		while (((n & 1) == 0) && n > 1) { // While n is even and more than 1 
			n >>= 1;
			++res;
		}
		if (n == 1)
			return res;
		else
			return -1;	
	}
	
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
			for(BasicBlock::iterator BI = B.begin(); BI != B.end(); ) {
				Instruction& inst(*BI);
				bool inst_removed = false;	// Is this instruction removed due to constant folding?
				
				// In binary operations
				if(BinaryOperator::classof(&inst)) {
					BinaryOperator *bop((BinaryOperator*)&inst);
					Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
					Value *val2(bop->getOperand(1));
					if(!ConstantInt::classof(val1) || !ConstantInt::classof(val2)) {++BI; continue;} // The values are not integers, then there's nothing to do!
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
			// Iterate over all instructions
			for(BasicBlock::iterator BI = B.begin(), BE = B.end(); BI != BE; ) {
				Instruction& inst(*BI);
				bool inst_removed = false;	// Is this instruction removed due to strength reduction
				
				// In binary operations
				if(BinaryOperator::classof(&inst)) {
					BinaryOperator *bop((BinaryOperator*)&inst);
					Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
					Value *val2(bop->getOperand(1));

					switch (bop->getOpcode()) {	// Switch on the operator
					
					case Instruction::Mul: 	//TODO: Figure out how to replace x*3 or 3*x by x<<1 + x
						// x * 2^k, 2^k * x ==> (x << k)
						if (ConstantInt::classof(val1)) {		// Make sure the constant value, if any, is val2
							Value *t(val1);
							val1 = val2;
							val2 = t;
						}
						
						if (ConstantInt::classof(val2)) {	// Now if it is really a constant
							ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);
							int64_t log_i = find_log(ci2->getSExtValue());
							if (log_i >= 0) {	// We return -1 if the constant is not a power of 2
								BinaryOperator *modified_inst(BinaryOperator::Create(Instruction::Shl, val1, get_const(ci2->getType(), log_i)));
								DBG(outs() << "Replacing: " << val1->getName() << " * " << ci2->getSExtValue() << " with " << val1->getName() << " << " << log_i << "\n");
								ReplaceInstWithInst(B.getInstList(),BI,modified_inst);
								LocalOptsInfo.numStrengthReds++;
							}
						}
						break;

					case Instruction::SDiv:
						// x / 2^k ==> (x >> k)
						if (ConstantInt::classof(val2)) {	// If val2 is a constant
							ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);
							int64_t log_i = find_log(ci2->getSExtValue());
							if (log_i >= 0) {	// If it is a power of 2
								BinaryOperator *modified_inst(BinaryOperator::Create(Instruction::LShr, val1, get_const(ci2->getType(), log_i)));
								DBG(outs() << "Replacing: " << val1->getName() << " / " << ci2->getSExtValue() << " with " << val1->getName() << " >> " << log_i << "\n");
								ReplaceInstWithInst(B.getInstList(),BI,modified_inst);
								LocalOptsInfo.numStrengthReds++;
							}
						}
						break;

					}
				}
				
				// Increment the iterator only if the current instruction was not removed. 
				// This is chosen instead of doing --BI when we remove the instruction because --BI crashes when BI is at B.begin()
				if (!inst_removed) ++BI;	
			}
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
