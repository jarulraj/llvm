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

#define get_const(t,val) ConstantInt::get((IntegerType*)t,val)
#define DEBUG 1

#ifdef DEBUG
	#define DBG(a) a
#else
	#define DBG(a) 
#endif

namespace
{
	
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
				bool inst_removed = false;
				
				// In binary operations
				if(BinaryOperator::classof(&inst)) {
					BinaryOperator *bop((BinaryOperator*)&inst);
					Value *val1(bop->getOperand(0));
					Value *val2(bop->getOperand(1));
					if(!ConstantInt::classof(val1) || !ConstantInt::classof(val2)) continue; // The values are not integers
					ConstantInt *ci1 = dyn_cast<ConstantInt>(val1);
					ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);

					switch (bop->getOpcode()) {
					
					case Instruction::Add: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) + (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " + " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::Sub: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) - (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " - " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;
						
					case Instruction::Mul: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) * (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " * " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::SDiv: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) / (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " / " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::Shl: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) << (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " << " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;

					case Instruction::LShr: {
						ConstantInt *ci_final(get_const(ci1->getType(), (ci1->getSExtValue()) >> (ci2->getSExtValue())));
						DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " >> " << ci2->getSExtValue() << " = " << ci_final->getSExtValue() << "\n");
						ReplaceInstWithValue(B.getInstList(),BI,ci_final);
						inst_removed = true;
						LocalOptsInfo.numConstantFolds++;
						}
						break;
					}
				}
				
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
					DBG(B.dump());
					algebraic_optimizations(B);
					constant_folding(B);
					strength_reduction(B);
					B.dump();
				}
			}
			
			printLocalOptsSummary();
			return true;
		}
	};

	char LocalOpts::ID = 0;
	RegisterPass<LocalOpts> X("local-opts", "15745: Local Optimizations");
}
