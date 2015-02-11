// 15-745 S15 Assignment 2: available.cpp
// Group: jarulraj, nkshah
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

using namespace llvm;

namespace {
    class AvailableExpressions : public FunctionPass {

    public:
        static char ID;

        AvailableExpressions() : FunctionPass(ID)
        {
            // Setup the pass
            Direction direction = Direction::FORWARD;
            MeetOp meet_op = MeetOp::INTERSECTION;

            pass = AEAnalysis(direction, meet_op);
        }

    private:

        // AE Analysis class
        class AEAnalysis : public DataFlow {
        public:

            AEAnalysis() : DataFlow(Direction::INVALID_DIRECTION, MeetOp::INVALID_MEETOP) {	}
            AEAnalysis(Direction direction, MeetOp meet_op) : DataFlow(direction, meet_op) { }

        protected:

            TransferOutput transferFn(BitVector input, std::map<void*, int> domainToIndex, BasicBlock* block)
            {
                TransferOutput transferOutput;


                return transferOutput;
            }

        };

        // The pass
        AEAnalysis pass;

        virtual bool runOnFunction(Function &F) {
            // Print Information
            std::string function_name = F.getName();
            DBG(outs() << "FUNCTION :: " << function_name  << "\n");
            DataFlowResult output;

            // Setup the pass
            std::vector<void*> domain;

            // Here's some code to familarize you with the Expression
            // class and pretty printing code we've provided:

            vector<Expression> expressions;
            for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
                BasicBlock* block = FI;
                for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
                    Instruction * I = i;

                    // We only care about available expressions for BinaryOperators
                    if (BinaryOperator * BI = dyn_cast<BinaryOperator>(I)) {
                        // Create a new Expression to capture the RHS of the BinaryOperator
                        expressions.push_back(Expression(BI));
                    }


                }
            }

            // Print out the expressions used in the function
            outs() << "Expressions used by this function:\n";
            printSet(&expressions);

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
                // Could also use getShortValueName((Value*) element)
                DBG(outs() << "Element : " << *((Value*) element) << "\n");
            }
            DBG(outs() << "------------------------------------------\n\n");

            // For LVA, both are empty sets
            BitVector boundaryCond(domain.size(), false);
            BitVector initCond(domain.size(), false);
            std::stringstream ss;

            // Apply pass
            output = pass.run(F, domain, boundaryCond, initCond);
            //printResult(output);

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

    char AvailableExpressions::ID = 0;
    RegisterPass<AvailableExpressions> X("available",
                                         "15745 Available Expressions");
}
