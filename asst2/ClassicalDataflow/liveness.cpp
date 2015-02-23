// 15-745 S15 Assignment 2: liveness.cpp
// Group: jarulraj, nkshah
////////////////////// //////////////////////////////////////////////////////////

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

            LivenessAnalysis() : DataFlow(Direction::INVALID_DIRECTION, MeetOp::INVALID_MEETOP) {	}
            LivenessAnalysis(Direction direction, MeetOp meet_op) : DataFlow(direction, meet_op) { }

        protected:
            TransferOutput transferFn(BitVector input,  std::vector<void*> domain, std::map<void*, int> domainToIndex, BasicBlock* block)
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
                                // neighborVals has no mapping for this block, then create one
                                if (transferOutput.neighborVals.find(valBlock) == transferOutput.neighborVals.end())
                                    transferOutput.neighborVals[valBlock] = BitVector(domainSize);

                                int valInd = domainToIndex[(void*)val];
                                // Set the bit corresponding to "val"
                                transferOutput.neighborVals[valBlock].set(valInd);

                            }
                        }
                    }

                    //Non-phi nodes: Simply add operands to the use set
                    else {
                        for (User::op_iterator opnd = insn->op_begin(), opE = insn->op_end(); opnd != opE; ++opnd) {
                            Value* val = *opnd;
                            if (isa<Instruction>(val) || isa<Argument>(val)) {
                                int valInd = domainToIndex[(void*)val];

                                // Add to useSet only if not already defined in the block somewhere earlier
                                if (!defSet[valInd])
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
                // Complement of defSet
                transferOutput.element.flip();
                // input - defSet = input INTERSECTION Complement of defSet
                transferOutput.element &= input;
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

            // PRINTING RESULTS

            // We use the results to compute the final liveness (we handle phi nodes here)
            std::stringstream ss;

            for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
                BasicBlock* block = BI;

                // liveness at OUT
                BitVector liveValues = output.result[block].out;

                // Generate Print Information in Reverse Order
                std::vector<std::string> revOut;

                revOut.push_back("//===--------------------------------------------------------------------------------------------------------------------------===//");

                // Print live variables at the end of the block
                ss.clear();
                ss.str(std::string());
                ss << std::setw(WIDTH) << std::right;
                ss << printSet(domain, liveValues, 0);
                revOut.push_back(ss.str());

                // Iterate backward through the block, update liveness
                for (BasicBlock::reverse_iterator insn = block->rbegin(), IE = block->rend(); insn != IE; ++insn) {

                    // Add the instruction itself
                    revOut.push_back(std::string(WIDTH, ' ') + printValue(&*insn));

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
                        ss.clear();
                        ss.str(std::string());
                        ss << std::setw(WIDTH) << std::right;
                        ss << printSet(domain, liveValues, 0);
                        revOut.push_back(ss.str());
                    }
                }

                revOut.push_back("//===--------------------------------------------------------------------------------------------------------------------------===//");

                // Since we added strings in the reverse order, print them in reverse
                for (std::vector<std::string>::reverse_iterator it = revOut.rbegin(); it != revOut.rend(); ++it)
                        outs() << *it << "\n";
            }

            // No modification
            return false;
        }

        virtual bool runOnModule(Module& M) {
            bool modified = false;

            // Run analysis on each function
            for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
                bool function_modified = false;

                function_modified = runOnFunction(*MI);

                modified |= function_modified;
            }

            return modified;
        }

    };

    char Liveness::ID = 0;
    RegisterPass<Liveness> X("liveness", "15745 Liveness");

}
