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
//#define DEBUG 1

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
                if (n <= 0)
                        return -1;
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

        // If n is (a power of 2) +- 1, then set that power in the exponent argument and return n-that_power_of_2.
        // Else, return -2.
        int64_t find_log_improved (int64_t n, int64_t &exponent)
        {
                if (n <= 0)
                        return -1;
                int64_t res = -2;
                if ((exponent = find_log(n)) >= 0) {
                        return 0;
                }
                else if ((exponent = find_log(n-1)) >= 0) {
                        return 1;
                }
                else if ((exponent = find_log(n+1)) >= 0) {
                        return -1;
                }
                else {
                        return -2;
                }
        }

        class LocalOpts : public ModulePass
        {

                // Algebraic Optimizations
                bool algebraic_optimizations(BasicBlock& B)
                {
                        bool modified = false;

                        // Iterate over all instructions
                        for(BasicBlock::iterator BI = B.begin(); BI != B.end(); )
                        {
                                Instruction& inst(*BI);
                                bool inst_removed = false;   // Is inst removed ?

                                // In binary operations
                                if(BinaryOperator *bop = dyn_cast<BinaryOperator>(&inst))
                                {
                                        Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
                                        Value *val2(bop->getOperand(1));
                                        IntegerType *itype = dyn_cast<IntegerType>(BI->getType());

                                        // Skip non-Integer types
                                        if(itype == NULL)
                                        {
                                                BI++;
                                                continue;
                                        }

                                        const APInt ap_int_zero = APInt(itype->getBitWidth(), 0);
                                        const APInt ap_int_one  = APInt(itype->getBitWidth(), 1);

                                        ConstantInt *ci;
                                        ConstantInt *ci_zero = ConstantInt::get(val1->getContext(), ap_int_zero);
                                        ConstantInt *ci_one  = ConstantInt::get(val1->getContext(), ap_int_one);

                                        switch (bop->getOpcode()) // Fold depending on what operator is used
                                        {

                                        case Instruction::Add:  // Addition
                                                // 0 + x = x
                                                if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 + x = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val2);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }

                                                }
                                                // x + 0 = x
                                                else if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x + 0 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }

                                                break;

                                        case Instruction::Sub:  // Subtraction
                                                // x - 0 = x
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x - 0 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }

                                                }
                                                //  x - x = 0
                                                else if(val1 == val2)
                                                {
                                                        DBG(outs()<<"AlgebraicIdentities :: x - x = 0 \n");
                                                        ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                        inst_removed = true;
                                                        LocalOptsInfo.numAlgebraicOpts++;
                                                }

                                                break;

                                        case Instruction::Mul:  // Multiplication
                                                // x * 1 = x, x * 0 = 0
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_one))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x * 1 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                        else if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x * 0 = 0 \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                // 1 * x = x, 0 * x = 0
                                                else if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_one))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 1 * x = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val2);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                        else if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 * x = 0 \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }

                                                }

                                                break;

                                        case Instruction::UDiv:  // Division
                                        case Instruction::SDiv:
                                                // x / 1 = x
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_one))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x / 1 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }

                                                }
                                                //  0 / x = 0
                                                else if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 / x = 0 \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                //  x / x = 1
                                                else if(val1 == val2)
                                                {
                                                        DBG(outs()<<"AlgebraicIdentities :: x / x = 1 \n");
                                                        ReplaceInstWithValue(B.getInstList(), BI, ci_one);
                                                        inst_removed = true;
                                                        LocalOptsInfo.numAlgebraicOpts++;
                                                }

                                                break;

                                        case Instruction::And:  // And
                                                // x && 0 = 0
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x && 0 = 0 \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                // 0 && x = 0
                                                else if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 && x = 0 \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, ci_zero);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }

                                                break;

                                        case Instruction::Or:  // Or
                                                // x || 0 = x
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x || 0 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                // 0 || x = x
                                                else if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 || x = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val2);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                break;

                                        case Instruction::Xor:  // Xor
                                                // x ^ 0 = x
                                                if(ConstantInt::classof(val2))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val2);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: x ^ 0 = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val1);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }
                                                // 0 ^ x = x
                                                else if(ConstantInt::classof(val1))
                                                {
                                                        ci = dyn_cast<ConstantInt>(val1);
                                                        if(ci->getValue().eq(ap_int_zero))
                                                        {
                                                                DBG(outs()<<"AlgebraicIdentities :: 0 ^ x = x \n");
                                                                ReplaceInstWithValue(B.getInstList(), BI, val2);
                                                                inst_removed = true;
                                                                LocalOptsInfo.numAlgebraicOpts++;
                                                        }
                                                }

                                                break;



                                        default:
                                                break;
                                        }
                                }

                                // Increment the iterator only if the current instruction was not removed.
                                if(!inst_removed)
                                        BI++;

                                // Modified block
                                if(inst_removed)
                                        modified = true;
                        }

                        return modified;
                }

                // Constant Folding
                bool constant_folding(BasicBlock& B)
                {
                        bool modified = false;

                        // Iterate over all instructions
                        for(BasicBlock::iterator BI = B.begin(); BI != B.end(); )
                        {
                                Instruction& inst(*BI);
                                bool known_inst = true;
                                bool inst_removed = false;	// Is this instruction removed

                                // In binary operations
                                if(BinaryOperator *bop = dyn_cast<BinaryOperator>(&inst))
                                {
                                        Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
                                        Value *val2(bop->getOperand(1));

                                        if(!ConstantInt::classof(val1) || !ConstantInt::classof(val2)) {
                                                // The values are not const integers, then there's nothing to do!
                                                ++BI;
                                                continue;
                                        }

                                        ConstantInt *ci1 = dyn_cast<ConstantInt>(val1);	// Get constants from values
                                        ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);
                                        ConstantInt *ci_final;

                                        switch (bop->getOpcode()) // Fold depending on what operator is used
                                        {

                                        case Instruction::Add:  // Addition
                                                ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) + (ci2->getSExtValue()));
                                                break;
                                        case Instruction::Sub: // Subtraction
                                                ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) - (ci2->getSExtValue()));
                                                break;
                                        case Instruction::Mul: // Multiplication
                                                ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) * (ci2->getSExtValue()));
                                                break;
                                        case Instruction::UDiv: // Division
                                        case Instruction::SDiv:
                                                if(ci2->getSExtValue() != 0)
                                                        ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) / (ci2->getSExtValue()));
                                                else
                                                        known_inst = false; // Divide by zero
                                                break;
                                        case Instruction::Shl:  // Left Shift
                                                ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) << (ci2->getSExtValue()));
                                                break;
                                        case Instruction::LShr: // (Logical) Right Shift
                                                ci_final = get_const(ci1->getType(), (ci1->getSExtValue()) >> (ci2->getSExtValue()));
                                                break;

                                        default:
                                                // Unknown instruction
                                                known_inst = false;
                                                break;
                                        }

                                        // Known instruction
                                        if(known_inst)
                                        {
                                                DBG(outs() << "ConstantFolding: " << ci1->getSExtValue() << " " << bop->getOpcodeName() << " "  << ci2->getSExtValue()
                                                    << " = " << ci_final->getSExtValue() << "\n");
                                                ReplaceInstWithValue(B.getInstList(),BI,ci_final);
                                                LocalOptsInfo.numConstantFolds++;
                                                inst_removed = true;
                                        }

                                }

                                // Increment the iterator only if the current instruction was not removed.
                                // This is chosen instead of doing --BI when we remove the instruction because --BI crashes when BI is at B.begin()
                                if (!inst_removed)
                                        ++BI;

                                // Modified block
                                if(inst_removed)
                                        modified = true;

                        }

                        return modified;
                }

                // Strength Reduction
                bool strength_reduction(BasicBlock& B)
                {
                        bool modified = false;

                        // Iterate over all instructions
                        for(BasicBlock::iterator BI = B.begin(), BE = B.end(); BI != BE; ) {
                                Instruction& inst(*BI);
                                bool inst_removed = false;	// Is this instruction removed due to strength reduction

                                // In binary operations
                                if(BinaryOperator *bop = dyn_cast<BinaryOperator>(&inst))
                                {
                                        Value *val1(bop->getOperand(0));	// Get the first and the second operand (as values)
                                        Value *val2(bop->getOperand(1));

                                        switch (bop->getOpcode()) {	// Switch on the operator

                                        case Instruction::Mul:
                                                // x * 2^k, 2^k * x ==> (x << k)
                                                if (ConstantInt::classof(val1)) {		// Make sure the constant value, if any, is val2
                                                        Value *t(val1);
                                                        val1 = val2;
                                                        val2 = t;
                                                }

                                                if (ConstantInt::classof(val2)) {	// Now if it is really a constant
                                                        ConstantInt *ci2 = dyn_cast<ConstantInt>(val2);
                                                        int64_t log_i = 0;
                                                        int64_t check_pow = find_log_improved(ci2->getSExtValue(),log_i);
                                                        if (check_pow != -2) {	// if val2 is 2^k +/- 1
                                                                BinaryOperator *modified_inst(BinaryOperator::Create(Instruction::Shl, val1, get_const(ci2->getType(), log_i)));
                                                                if (check_pow == 0) { // val2 is 2^k --> only need one instruction
                                                                        DBG(outs() << "Replacing: " << val1->getName() << " * " << ci2->getSExtValue() << " with " << val1->getName() << " << " << log_i << "\n");
                                                                        ReplaceInstWithInst(B.getInstList(),BI,modified_inst);
                                                                }
                                                                else if (check_pow == 1) {	// 2^k + 1
                                                                        DBG(outs() << "Replacing: " << val1->getName() << " * " << ci2->getSExtValue() << " with " << val1->getName() << " << " << log_i << " + " << val1->getName() << "\n");
                                                                        B.getInstList().insert(BI,modified_inst);
                                                                        BinaryOperator *final_inst(BinaryOperator::Create(Instruction::Add, modified_inst, val1));
                                                                        ReplaceInstWithInst(B.getInstList(),BI,final_inst);
                                                                }
                                                                else { // 2^k - 1
                                                                        DBG(outs() << "Replacing: " << val1->getName() << " * " << ci2->getSExtValue() << " with " << val1->getName() << " << " << log_i << " - " << val1->getName() << "\n");
                                                                        B.getInstList().insert(BI,modified_inst);
                                                                        BinaryOperator *final_inst(BinaryOperator::Create(Instruction::Sub, modified_inst, val1));
                                                                        ReplaceInstWithInst(B.getInstList(),BI,final_inst);
                                                                }
                                                                LocalOptsInfo.numStrengthReds++;
                                                        }
                                                }
                                                break;

                                        case Instruction::UDiv: // Division
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

                                        default:
                                                // Unhandled instruction
                                                break;
                                        }
                                }

                                // Increment the iterator only if the current instruction was not removed.
                                if (!inst_removed)
                                        ++BI;

                                // Modified block
                                if(inst_removed)
                                        modified = true;
                        }

                        return modified;
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

                // We only do local optimizations, so we don't modify the CFG.
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

                                        // Loop till block is modified
                                        while(1)
                                        {
                                                bool modified = false;

                                                // Algebraic optimization pass
                                                modified = modified || algebraic_optimizations(B);
                                                if(modified)
                                                {
                                                        DBG(B.dump());
                                                        DBG(outs() << "-------------------------------\n");
                                                }

                                                // Constant folding pass
                                                modified = modified || constant_folding(B);
                                                if(modified)
                                                {
                                                        DBG(B.dump());
                                                        DBG(outs() << "-------------------------------\n");
                                                }

                                                // Strength reduction pass
                                                modified = modified || strength_reduction(B);
                                                if(modified)
                                                {
                                                        DBG(B.dump());
                                                        DBG(outs() << "-------------------------------\n");
                                                }

                                                if(!modified)
                                                        break;
                                        };

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
