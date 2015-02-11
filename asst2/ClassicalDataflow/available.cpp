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

            TransferOutput transferFn(BitVector input, std::vector<void*> domain, std::map<void*, int> domainToIndex, BasicBlock* block)
            {
                TransferOutput transferOutput;

                // Calculating the set of expressions generated and killed in BB
                int domainSize = domainToIndex.size();
                BitVector GenSet(domainSize);
                BitVector KillSet(domainSize);

                for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
                    Instruction * I = i;
                    // We only care about available expressions for BinaryOperators
                    if (BinaryOperator * BI = dyn_cast<BinaryOperator>(I)) {
                        // Create a new Expression to capture the RHS of the BinaryOperator
                        Expression *expr = new Expression(BI);
                        Expression *match = NULL;
                        bool found = false;

                        for(void* element : domain)
                        {
                            if((*expr) == *((Expression *) element))
                            {
                                found = true;
                                match = (Expression *) element;
                                break;
                            }
                        }

                        // Generated expression
                        if(found)
                        {
                            int valInd = domainToIndex[(void*)match];

                            // The instruction definitely evaluates the expression in RHS here
                            // The expression  will be killed if one of its operands is
                            // redefined subsequently in the BB.
                            GenSet.set(valInd);
                        }
                    }

                    // Killed expressions

                    // The assignment kills all expressions in which the LHS is an operand.
                    // They will be generated if subsequently recomputed in BB.
                    StringRef insn  =  I->getName();
                    if(!insn.empty())
                    {
                        //DBG(outs() << "Insn : " << insn  << "\n");
                        for(auto domain_itr = domain.begin() ; domain_itr != domain.end() ; domain_itr++)
                        {
                            Expression* expr = (Expression*) (*domain_itr);

                            StringRef op1 = expr->v1->getName();
                            StringRef op2 = expr->v2->getName();

                            if(op1.equals(insn) || op2.equals(insn))
                            {
                                //DBG(outs() << "Expr : " << expr->toString()  << " ");
                                // Kill if either operand 1 or 2 match the variable assigned
                                std::map<void*, int>::iterator iter = domainToIndex.find((void*) expr);

                                if (iter != domainToIndex.end())
                                {
                                    //DBG(outs() << "Index : " << (*iter).second  << "\n");
                                    KillSet.set((*iter).second);
                                }
                            }
                        }
                    }

                }

                //printBitVector(GenSet);
                //printBitVector(KillSet);
                // Transfer function = GenSet U (input - KillSet)

                transferOutput.element = KillSet;
                // Complement of KillSet
                transferOutput.element.flip();
                // input - KillSet = input INTERSECTION Complement of KillSet
                transferOutput.element &= input;
                transferOutput.element |= GenSet;

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

            // Compute the domain

            for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
                BasicBlock* block = FI;
                for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
                    Instruction * I = i;

                    // We only care about available expressions for BinaryOperators
                    if (BinaryOperator * BI = dyn_cast<BinaryOperator>(I)) {

                        // Create a new Expression to capture the RHS of the BinaryOperator
                        Expression *expr = new Expression(BI);
                        bool found = false;

                        for(void* element : domain)
                        {
                            if((*expr) == *((Expression *) element))
                            {
                                found = true;
                                break;
                            }
                        }

                        if(found == false)
                            domain.push_back(expr);
                        else
                            delete expr;
                    }
                }
            }

            DBG(outs() << "------------------------------------------\n\n");
            DBG(outs() << "DOMAIN :: " << domain.size() << "\n");
            for(void* element : domain)
            {
                DBG(outs() << "Element : " << ((Expression*) element)->toString() << "\n");
            }
            DBG(outs() << "------------------------------------------\n\n");

            // For AEA, the boundary condition is phi and init condition is U.
            BitVector boundaryCond(domain.size(), false);
            BitVector initCond(domain.size(), true);

            // Apply pass
            output = pass.run(F, domain, boundaryCond, initCond);
            //printResult(output);

            // PRINTING RESULTS

            // Map domain values to index in bitvector
            std::map<void*, int> domainToIndex;
            for (int i = 0; i < domain.size(); i++)
                domainToIndex[(void*)domain[i]] = i;

            // We use the results to compute the available expressions
            std::stringstream ss;

            for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
                BasicBlock* block = BI;

                // AE at IN
                BitVector availableExprs = output.result[block].in;

                // Generate Print Information
                std::vector<std::string> out;

                out.push_back("//===--------------------------------------------------------------------------------------------------------------------------===//");

                // Print live variables at the entry of the block
                ss.clear();
                ss << std::setw(WIDTH) << std::right;
                ss.str(std::string());
                ss << printSet(domain, availableExprs, 1) << " :: BB Entry" ;
                out.push_back(ss.str());

                // Iterate forward through the block, updating availability of expressions
                for (BasicBlock::iterator insn = block->begin(), IE = block->end(); insn != IE; ++insn) {

                    // Add the instruction itself
                    out.push_back(std::string(WIDTH, ' ') + printValue(&*insn));

                    // Gen expressions
                    Instruction * I = insn;

                    // We only care about available expressions for BinaryOperators
                    if (BinaryOperator * BI = dyn_cast<BinaryOperator>(I)) {
                        // Create a new Expression to capture the RHS of the BinaryOperator
                        Expression *expr = new Expression(BI);
                        Expression *match = NULL;
                        bool found = false;

                        for(void* element : domain)
                        {
                            if((*expr) == *((Expression *) element))
                            {
                                found = true;
                                match = (Expression *) element;
                                break;
                            }
                        }

                        // Generated expression
                        if(found)
                        {
                            int valInd = domainToIndex[(void*)match];

                            // The instruction definitely evaluates the expression in RHS here
                            // The expression  will be killed if one of its operands is
                            // redefined subsequently in the BB.
                            availableExprs.set(valInd);
                        }
                    }

                    // Killed expressions

                    // The assignment kills all expressions in which the LHS is an operand.
                    // They will be generated if subsequently recomputed in BB.
                    StringRef insn_str  =  I->getName();

                    if(!insn_str.empty())
                    {
                        //DBG(outs() << "Insn : " << insn_str  << "\n");

                        for(auto domain_itr = domain.begin() ; domain_itr != domain.end() ; domain_itr++)
                        {
                            Expression* expr = (Expression*) (*domain_itr);

                            StringRef op1 = expr->v1->getName();
                            StringRef op2 = expr->v2->getName();

                            if(op1.equals(insn_str) || op2.equals(insn_str))
                            {
                                //DBG(outs() << "Expr : " << expr->toString()  << " ");

                                // Kill if either operand 1 or 2 match the variable assigned
                                std::map<void*, int>::iterator iter = domainToIndex.find((void*) expr);

                                if (iter != domainToIndex.end())
                                {
                                    //DBG(outs() << "Index : " << (*iter).second  << "\n");
                                    availableExprs.reset((*iter).second);
                                }
                            }
                        }
                    }

                    //printBitVector(availableExprs);

                    // Print live variables at this program point
                    PHINode* phiInst = dyn_cast<PHINode>(&*insn);
                    // Skip printing for phi instructions
                    if(phiInst == NULL)
                    {
                        ss.clear();
                        ss.str(std::string());
                        ss << std::setw(WIDTH) << std::right;
                        ss << printSet(domain, availableExprs, 1);
                        out.push_back(ss.str());
                    }
                }

                out.push_back("//===--------------------------------------------------------------------------------------------------------------------------===//");

                // Print strings
                for (std::vector<std::string>::iterator it = out.begin(); it != out.end(); ++it)
                    outs() << *it << "\n";

            }

            // No modification
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
