// 15-745 S15 Assignment 3: DeadCodeElimination.cpp
// Group: jarulraj, nkshah
////////////////////// //////////////////////////////////////////////////////////

#include "dataflow.h"

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/PostOrderIterator.h"

using namespace llvm;

namespace {

    class DCE : public FunctionPass {
    public:
        static char ID;

        DCE() : FunctionPass(ID) {

            // Setup the pass
            Direction direction = Direction::BACKWARD;
            MeetOp meet_op = MeetOp::UNION;

            pass = DCEAnalysis(direction, meet_op);
        }

        virtual void getAnalysisUsage(AnalysisUsage& AU) const {
            AU.setPreservesAll();
        }

    private:

        // DCE Analysis class
        class DCEAnalysis : public DataFlow {
        public:

            DCEAnalysis() : DataFlow(Direction::INVALID_DIRECTION, MeetOp::INVALID_MEETOP) {	}
            DCEAnalysis(Direction direction, MeetOp meet_op) : DataFlow(direction, meet_op) { }

        protected:
            TransferOutput transferFn(BitVector input,  std::vector<void*> domain, std::map<void*, int> domainToIndex, BasicBlock* block)
            {
                TransferOutput transferOutput;

                // Calculating the set of locally exposed uses
                int domainSize = domainToIndex.size();
                BitVector defSet(domainSize);
                BitVector useSet(domainSize);

                // First initialize
                useSet = input;

                // Iterate forward through the block to find live insns
                for (BasicBlock::iterator insn = block->begin(); insn != block->end(); ++insn) {
                    Instruction *I = &(*insn);

                    if(isa<TerminatorInst>(I) ||  isa<DbgInfoIntrinsic>(I) ||
                       isa<LandingPadInst>(I) ||  I->mayHaveSideEffects())
                    {
                        std::map<void*, int>::iterator iter = domainToIndex.find((void*)I);

                        if(iter != domainToIndex.end())
                        {
                            int valInd = domainToIndex[(void*)I];

                            DBG(outs() << "Marking live instruction :: " << printValue(I) << "\n");

                            useSet.set(valInd);
                        }
                    }
                }

                // Iterate backward through the block, update SLV
                for (BasicBlock::reverse_iterator insn = block->rbegin(); insn != block->rend(); ++insn) {

                    // Alter data flow only if insn itself is strongly live
                    std::map<void*, int>::iterator iter = domainToIndex.find((void*)(&(*insn)));

                    if(iter != domainToIndex.end())
                    {
                        int valInd = domainToIndex[(void*)(&(*insn))];
                        if(useSet[valInd] == false)
                            continue;

                        // Phi nodes: add operands to the list we store in transferOutput
                        if (PHINode* phi_insn = dyn_cast<PHINode>(&*insn)) {
                            for (int ind = 0; ind < phi_insn->getNumIncomingValues(); ind++) {

                                Value* val = phi_insn->getIncomingValue(ind);

                                if (isa<Instruction>(val)) {
                                    BasicBlock* valBlock = phi_insn->getIncomingBlock(ind);

                                    // neighborVals has no mapping for this block, then create one
                                    if (transferOutput.neighborVals.find(valBlock) == transferOutput.neighborVals.end())
                                        transferOutput.neighborVals[valBlock] = BitVector(domainSize);

                                    int valInd = domainToIndex[(void*)val];

                                    DBG(outs() << "Marking phi operand :: " << printValue(val) << "\n");

                                    // Set the bit corresponding to "val"
                                    transferOutput.neighborVals[valBlock].set(valInd);
                                }
                            }
                        }
                        //Non-phi nodes: Simply add operands to the use set
                        else {
                            for (User::op_iterator opnd = insn->op_begin(), opE = insn->op_end(); opnd != opE; ++opnd) {
                                Value* val = *opnd;

                                if (isa<Instruction>(val)) {
                                    int valInd = domainToIndex[(void*)val];

                                    DBG(outs() << "Marking operand :: " << printValue(val) << "\n");

                                    useSet.set(valInd);
                                }
                            }
                        }

                        // Definitions
                        iter = domainToIndex.find((void*)&(*insn));
                        if (iter != domainToIndex.end())
                            defSet.set((*iter).second);
                    }
                }

                // Transfer function = useSet U (input - defSet)

                transferOutput.element = defSet;
                // Complement of defSet
                transferOutput.element.flip();
                // input - defSet = input INTERSECTION Complement of defSet
                transferOutput.element &= input;
                transferOutput.element |= useSet;

                DBG(outs() << "\n\n--------------------------------------------------\n\n");

                return transferOutput;
            }

        };

        // The pass
        DCEAnalysis pass;

        bool runOnFunction(Function &F) {
            // Print Information
            std::string function_name = F.getName();
            DBG(outs() << "FUNCTION :: " << function_name  << "\n");
            DataFlowResult output;

            // Setup the pass
            std::vector<void*> domain;

            // Compute domain for function
            for(inst_iterator II = inst_begin(F), IE = inst_end(F); II!=IE; ++II) {
                Instruction& insn(*II);

                if(std::find(domain.begin(),domain.end(),(&(*II))) == domain.end())
                    domain.push_back((void*)(&(*II)));
            }

            DBG(outs() << "------------------------------------------\n\n");
            DBG(outs() << "DOMAIN :: " << domain.size() << "\n");
            for(void* element : domain)
            {
                DBG(outs() << "Element : " << *((Value*) element) << "\n"); // Could also use getShortValueName((Value*) element)
            }
            DBG(outs() << "------------------------------------------\n\n");

            // For SLVA, both are empty sets
            BitVector boundaryCond(domain.size(), false);
            BitVector initCond(domain.size(), false);
            bool modified = false;

            // Apply pass
            output = pass.run(F, domain, boundaryCond, initCond);
            printResult(output);

            // Prepare an order in which we will traverse BasicBlocks.
            for (po_iterator<BasicBlock*> BI = po_begin(&F.getEntryBlock()), BE = po_end(&F.getEntryBlock()); BI != BE; ++BI) {
                BasicBlock* block = *BI;

                // strong liveness at IN
                BitVector SLV = output.result[block].in;
                printBitVector(SLV);
                std::vector<Instruction *> deleteSet;

                // Figure out instructions to delete
                for (BasicBlock::reverse_iterator insn = BI->rbegin(); insn != BI->rend(); ++insn) {
                    Instruction* I = &(*insn);

                    // Check if insn is live
                    if(isa<TerminatorInst>(I) ||  isa<DbgInfoIntrinsic>(I) ||
                       isa<LandingPadInst>(I) ||  I->mayHaveSideEffects())
                        continue;

                    // Check if use empty
                    //if(!I->use_empty())
                    //    continue;

                    if(std::find(domain.begin(), domain.end(), I) != domain.end())
                    {
                        int valInd = output.domainToIndex[(void*)I];

                        if(SLV[valInd] == false)
                        {
                            DBG(outs() << "Going to delete instruction :: " << printValue(I) << "\n");
                            deleteSet.push_back(I);
                            modified = true;
                        }
                    }
                }

                // do actual deletion
                for(auto I : deleteSet){
                    DBG(outs() << "Deleting instruction :: " << printValue(I) << "\n");
                    I->eraseFromParent();
                }

            }

            // Potential modification
            return modified;
        }

    };

    char DCE::ID = 0;
    RegisterPass<DCE> X("dead-code-elimination", "15745 Dead Code Elimination");
}
